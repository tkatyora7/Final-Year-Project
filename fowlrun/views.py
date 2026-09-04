from datetime import datetime  
from rest_framework.decorators import api_view
from rest_framework.response import Response
from .serializers import FowlRunConditionSerializer,ImageSerializer
from django.conf import settings
from rest_framework import status
from main_application.models import FowlRunConditions ,Alerts
import json
from django.views.decorators.csrf import csrf_exempt
from rest_framework.decorators import api_view
from rest_framework.response import Response
from main_application.models import AudioFeed
import wave
import io
import base64
from django.core.files.base import ContentFile
from django.conf import settings
import cv2
import numpy as np
from main_application.models import *
from main_application.forms import *
import tensorflow as tf
from django.shortcuts import render, redirect
from PIL import Image
import numpy as np
from io import BytesIO
import tensorflow as tf
from tensorflow.keras.preprocessing import image
import numpy as np
import os
from .models import *
from django.http import JsonResponse
import datetime
import json
from django.core.mail import send_mail
from django.conf import settings
from django.contrib.auth.decorators import login_required


from django.contrib.auth import get_user_model
User = get_user_model()

user = User.objects.filter(username='takudzwa').first()





@csrf_exempt
def device_ping(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            device_name = data.get('device_name')
            
            if device_name:
                device, created = DeviceStatus.objects.get_or_create(name=device_name)
                device.last_ping = timezone.now()
                device.is_online = True
                device.save()
                return JsonResponse({'status': 'success'})
            else:
                return JsonResponse({'status': 'error', 'message': 'Device name required'}, status=400)
                
        except Exception as e:
            return JsonResponse({'status': 'error', 'message': str(e)}, status=400)
    
    return JsonResponse({'status': 'error', 'message': 'POST method required'}, status=405)







@api_view(['POST', 'GET'])
def fowlrun_conditions(request):
    print('Connecting to the ESP32')

    try:
      
        if not request.data:
            data = json.loads(request.body.decode('utf-8'))
        else:
            data = request.data

        print('Parsed data:', data)

    
        provided_secret_key = data.get("secret_key")
        temp = data.get("temperature")
        hum = data.get("humidity")

        print('Secret key:', provided_secret_key)
        print('Temperature:', temp)
        print('Humidity:', hum)

      
        if not provided_secret_key or provided_secret_key != settings.ESP_32_SECRET_KEY:
            return Response({"error": "Invalid or missing secret key"}, status=status.HTTP_403_FORBIDDEN)

    except json.JSONDecodeError as e:
        print('JSON decode error:', e)
        return Response({"error": "Invalid JSON"}, status=status.HTTP_400_BAD_REQUEST)

   
    serializer = FowlRunConditionSerializer(data=data)
    if serializer.is_valid():
        serializer.save()
        return Response({"message": "Data captured successfully!"}, status=status.HTTP_201_CREATED)

    return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)





@csrf_exempt
@api_view(['POST'])
def upload_audio_analysis(request):
    if request.method == 'POST':
        try:
            provided_secret_key = request.headers.get('X-Secret-Key')
            if not provided_secret_key or provided_secret_key != settings.ESP_32_SECRET_KEY:
                return Response({"error": "Invalid or missing secret key"}, 
                                status=status.HTTP_403_FORBIDDEN)

            data = request.data
            sound_count = data.get("sound_count")
            sound_status = data.get("sound_status")
            risk_level = data.get("risk_level")
            prolong_silence  = data.get("prolong_silence")
            humidity  = data.get("humidity")
            temperature  = data.get("temperature")
            
            
            if sound_count is None or sound_status is None or risk_level is None:
                return Response({"error": "Missing required fields"}, status=status.HTTP_400_BAD_REQUEST)

            sound = SoundAnalysis(sound_count=sound_count,
                                  prolong_silence=prolong_silence,
                                  risk_level=risk_level,
                                  sound_status=sound_status,
                                  humidity = humidity,
                                  temperature= temperature
                                  )
                                  
           
            sound.save()
           
            return Response({
                "status": "success",
                "message": "Sound analysis data received",
                "data": {
                    "sound_count": sound_count,
                    "sound_status": sound_status,
                    "risk_level": risk_level,
                    "prolong_silence":prolong_silence,
                    "temperature":temperature,
                    "humidity":humidity
                    
                }
            }, status=status.HTTP_201_CREATED)

        except Exception as e:
            return Response({"error": str(e)}, status=status.HTTP_500_INTERNAL_SERVER_ERROR)



interpreter = tf.lite.Interpreter(model_path="/home/takudzwa/Documents/Projects/broiler_disease/final_year_broiler/fomo_model.tflite")
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

class_names = ['cocci', 'health', 'ncd' , 'others']  

def preprocess_image(image_file, target_size):
    try:
        img = Image.open(image_file)
        img = img.resize(target_size)
        img_array = np.array(img) / 255.0  
        img_array = np.expand_dims(img_array, axis=0).astype(np.float32) 
        return img_array
    except Exception as e:
        print(f"Error preprocessing image: {e}")
        return None


@api_view(['POST'])
def fowlrun_images(request):
    print(user.email)
    try:
        provided_secret_key = request.data.get("secret_key")
        if not provided_secret_key or provided_secret_key != settings.ESP_32_SECRET_KEY:
            return Response(
                {"error": "Invalid or missing secret key"}, 
                status=status.HTTP_403_FORBIDDEN
            )
        image_base64 = request.data.get('image')
        width = request.data.get('width')
        height = request.data.get('height')
        
        if not all([image_base64, width, height]):
            return Response(
                {"error": "Missing image data or dimensions"}, 
                status=status.HTTP_400_BAD_REQUEST
            )
        if ';base64,' in image_base64:
            format, imgstr = image_base64.split(';base64,') 
            ext = format.split('/')[-1]
            image_data = base64.b64decode(imgstr)
        else:
            image_data = base64.b64decode(image_base64)
            ext = 'jpg' 

        img_array = np.frombuffer(image_data, dtype=np.uint8)
        img = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
        
        if img is None:
            return Response(
                {"error": "Invalid image data"}, 
                status=status.HTTP_400_BAD_REQUEST
            )
            
        if img.shape[0] != int(height) or img.shape[1] != int(width):
            return Response(
                {"error": f"Dimension mismatch. Expected {width}x{height}, got {img.shape[1]}x{img.shape[0]}"}, 
                status=status.HTTP_400_BAD_REQUEST
            )
        image_file = ContentFile(image_data, name=f'capture.{ext}')
        
        data = {
            'image': image_file,
            'width': width,
            'height': height,
        }
        serializer = ImageSerializer(data=data)
        if serializer.is_valid():
            broiler_image=serializer.save()
            input_shape = (input_details[0]['shape'][1], input_details[0]['shape'][2])
        processed_image = preprocess_image(broiler_image.image, input_shape)

        if processed_image is not None:
            interpreter.set_tensor(input_details[0]['index'], processed_image)
            interpreter.invoke()
            output_data = interpreter.get_tensor(output_details[0]['index'])

            predicted_class_index = np.argmax(output_data) 
            predicted_probability = float(output_data[0][predicted_class_index])
            health_status = class_names[predicted_class_index]
            
            # INITIAL FLAGS
            requires_environment_check = False
            should_send_email = False
            email_subject = ""
            email_message = ""

          
            print("The Broiler is health status is  ", health_status, predicted_probability)
            if health_status in ['cocci', 'health', 'ncd']:
                if predicted_probability < 0.7:
                    print('now checking environment conditions since it have less that 0.7')
                    requires_environment_check = True
                if health_status in ['cocci', 'ncd','health'] and predicted_probability >= 0.7:
                    should_send_email = True
                    email_subject = f"Urgent: {health_status.upper()} detected in your Fowlrun 1"
                    email_message = (
                        f"Our system has detected {health_status.upper()} in your poultry with {predicted_probability*100:.1f}% confidence.\n\n"
                        "Recommended actions:\n"
                        f"Log In To The Web Application For Recomandations"
                    )

                if requires_environment_check:
                    try:
                        latest_conditions = FowlRunConditions.objects.latest('timestamp')
                        temperature = latest_conditions.temperature
                        humidity = latest_conditions.humidity
                        soundStatus= True
                        if temperature > 39.5 or temperature < 30.0:
                             
                             print('high')
                             tempStatus = 'high'
                        else:
                            print('low')
                            tempStatus= 'low'

                        if humidity > 70.0:
                             humidityStatus = 'high'
                        else:
                             humidityStatus= 'low'

                        if health_status == "ncd":
                            if soundStatus == "abnormal" and  tempStatus == "high" and  humidityStatus == "high":
                                diagnosis = "Possible New Castle detected, with environmental data available"
                                should_send_email = True
                                print('send email')
                                health_status = health_status
                            else:
                                should_send_email = True
                                diagnosis = "Possible New Castle detected, but  environmental data in not concusive"
                                health_status = f"uncertain_{health_status}"
                        
                        elif health_status == "cocci":
                            if soundStatus == "abnormal" and  tempStatus == "high":
                                    diagnosis = "Possible coccidiosis detected, with environmental data available"
                                    should_send_email = True
                                    health_status = {health_status}
                                    print('send email')
                            else:
                                diagnosis = "Possible coccidiosis detected, with environmental data available"
                                should_send_email = True
                                health_status = f"uncertain_{health_status}"
                        
                    except FowlRunConditions.DoesNotExist:
                        diagnosis = "Possible coccidiosis detected, but no environmental data available"
                       
               
                

            if health_status == 'health':
                is_health = True
            else:
                is_health = False
            alerts = Alerts(detection=health_status, is_health=is_health,probability=predicted_probability,detection_method='image')
            alerts.save()
            broiler_image.health_status = health_status
            broiler_image.save()
           

            if should_send_email and settings.EMAIL_HOST_USER:
                try:
                    print('sending email')
                    # send_mail(
                    #     email_subject,
                    #     email_message,
                    #     settings.EMAIL_HOST_USER,
                    #     [user.email],  
                    #     fail_silently=False,
                    # )
                except Exception as email_error:
                    alerts.email_sent = False
                    alerts.save()
                    print(f"Failed to send email: {str(email_error)}")

            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
    
        

        
    except Exception as e:
        return Response(
            {"error": f"Error processing image: {str(e)}"},
            status=status.HTTP_500_INTERNAL_SERVER_ERROR
        )
    
 

# @api_view(['POST'])
# @csrf_exempt
# def upload_audio(request):
#     if request.method == 'POST':
#         try:
           
#             provided_secret_key = request.headers.get('X-Secret-Key')
            
#             if not provided_secret_key or provided_secret_key != settings.ESP_32_SECRET_KEY:
#                 return Response({"error": "Invalid or missing secret key"}, 
#                               status=status.HTTP_403_FORBIDDEN)

        
#             raw_audio = request.body
            
         
#             timestamp = datetime.datetime.today()
#             filename = f"recording_{timestamp}.wav"
            
#             with io.BytesIO() as wav_buffer:
#                 with wave.open(wav_buffer, 'wb') as wav_file:
#                     wav_file.setnchannels(1)  
#                     wav_file.setsampwidth(2)  
#                     wav_file.setframerate(4000)  
#                     wav_file.writeframes(raw_audio)
                
#                 wav_buffer.seek(0)
#                 recording = AudioFeed.objects.create(
#                     audio_file=ContentFile(wav_buffer.getvalue(), name=filename),
#                     duration=len(raw_audio)/(4000*2), 
#                     sample_rate=4000
#                 )
            
#             return Response({
#                 'status': 'success',
#                 'url': recording.get_absolute_url(),
#                 'timestamp': recording.timestamp
#             }, status=status.HTTP_201_CREATED)
            
#         except Exception as e:
#             print(f"Error processing audio: {str(e)}")
#             return Response({'error': str(e)}, 
#                           status=status.HTTP_500_INTERNAL_SERVER_ERROR)
