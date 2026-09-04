from django.shortcuts import render , redirect
from django.contrib.auth import authenticate, login ,logout
from django.contrib.auth.decorators import login_required
from django.contrib import messages
from django.views.decorators.csrf import csrf_protect
from .models import BroilerImage , FowlRunConditions , Alerts ,AudioFeed
from .forms import BroilerImageForm ,BroilerAudioForm
import tensorflow as tf
from django.shortcuts import render, redirect
from PIL import Image
from io import BytesIO
from django.utils import timezone
import datetime
from tensorflow.keras.preprocessing import image
import numpy as np
import os
from fowlrun.models import DeviceStatus
from django.views.decorators.csrf import csrf_exempt
import json
from .models import *


#broiler infomation
broilers_count = BroilerImage.objects.all().count()
last_five_images = BroilerImage.objects.all().order_by('-id')[:5]
broilers = BroilerImage.objects.all()
latest_broilers =  BroilerImage.objects.last()

#condtions
fowlrun_temperature = 'Not Captured'
fowlrun_humidity = 'Not Captured'
fowlrun = 'No Condition'
timestamp='Not Recorded'

def update_function():
    try:
        latest_conditions = FowlRunConditions.objects.latest('timestamp')
        return (
            latest_conditions.temperature,
            latest_conditions.humidity,
            latest_conditions.fowlrun,
            latest_conditions.timestamp
        )
    except FowlRunConditions.DoesNotExist:
        return 'Not Captured', 'Not Captured', 'No Condition', 'Not Recorded'


def image_function():
    try:
        latest_broilers =  BroilerImage.objects.latest('timestamp')
        broilers_count = BroilerImage.objects.all().count()
        broilers =BroilerImage.objects.all()
        last_five_images = BroilerImage.objects.all().order_by('-id')[:5]
        return (
            latest_broilers,
            last_five_images,
            broilers,
            broilers_count
        )
    except BroilerImage.DoesNotExist:
        return 'No Images', 'Not Captured', 'No Images', 'No Images'
#AUTHENTICATION

@csrf_protect
def sign_in(request):
    if request.user.is_authenticated:
        return redirect('dashboard')  
        
    if request.method == 'POST':
        username = request.POST.get('username') 
        password = request.POST.get('password')  
        
        if username and password:  
            user = authenticate(request=request, username=username, password=password)
            if user is not None:
                if user.is_active:
                    login(request, user)  
                    messages.success(request, 'Successfully logged in!') 
                    return redirect('dashboard')  
                else:
                    messages.error(request, 'Your account is Inactive.')
            else:
                messages.error(request, 'Invalid username or password.')
        else:
            messages.error(request, 'Please fill in both username and password.')
            
    return render(request, 'login.html')

def signout(request):
    logout(request)
    messages.success(request,'Logged Out Succefully.')
    return redirect('login')

@login_required(login_url='login')
def dashboard(request):
    esp32, created = DeviceStatus.objects.get_or_create(name="ESP32")
    esp32_cam, created = DeviceStatus.objects.get_or_create(name="ESP32_CAM")
    
    # Update status based on last ping time
    esp32_online = esp32.update_status()
    esp32_cam_online = esp32_cam.update_status()
    fowlrun_temperature, fowlrun_humidity, fowlrun, timestamp = update_function()
    _, last_five_images, _,_ = image_function()
    print(esp32)
    print('online',esp32_online)

    recent_alerts = Alerts.objects.filter(
    detection__in=['cocci', 'health', 'ncd']).order_by('-id')[:5]

    context = {
        'temperature': fowlrun_temperature,  
        'humidity': fowlrun_humidity,     
        "broilers":broilers,
        "last_five_images":last_five_images,
        "fowlrun":fowlrun,
        "time_stamp": timestamp,
        "recent_alerts": recent_alerts,
        'esp32_online': esp32_online,
        'esp32_cam_online': esp32_cam_online,
        'last_ping': esp32.last_ping,           
        'esp32_cam_last_ping': esp32_cam.last_ping 
       
    }
    return render(request, 'main_dashboard.html', context)




# RECOMMENDATIONS
@login_required(login_url='login')
def recommendations(request):
    fowlrun_temperature, _, _, _ = update_function()
    latest_alert =  Alerts.objects.exclude(detection='others').latest('timestamp')
    latest_broilers =  BroilerImage.objects.exclude(health_status__in=['others','health']).latest('timestamp')
    context = {
        "temperature": fowlrun_temperature,
        "alert": latest_alert,
        "latest_broilers":latest_broilers
       
    }
    return render(request, 'recommendations.html', context)

@login_required(login_url='login')
def alerts(request):
    recent_alerts = recent_alerts = Alerts.objects.exclude(detection='others')
    # latest_alert = Alerts.objects.filter(detection__in=['cocci', 'health', 'ncd']).latest('timestamp')
    latest_alert=Alerts.objects.exclude(detection='others').latest('timestamp')
    
    context = {
        'recent_alerts':recent_alerts,
        "alert": latest_alert

  
        
    }
    return render(request, 'alerts.html', context)

#IMAGE PROCESSING
@login_required(login_url='login')
def camera_feed(request):
    latest_broiler, _, broilers,broiler_count = image_function()
    content = {
        "latest":latest_broiler,
        "broiler":broilers,
        "broiler_count":broiler_count
        
    }
    return render(request, 'camera_feed.html', content)



#IMAGE PROCESSING
@login_required(login_url='login')
def audio_feed(request):
    latest_data = SoundAnalysis.objects.last() 
    historical_data = SoundAnalysis.objects.all().order_by('-timestamp')[:20] 
    
    context = {
        'latest': latest_data,
        'historical_records': historical_data,
    }
    return render(request, 'audio_analysis.html', context)


#---------------------------------PREDICTION FROM FARMER HUB-------------------------------------------
# FOR IMAGE
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
    

def process_image_upload(request, image_form):
    try:
        # broiler_image = image_form.save(save=False)
        # if broiler_image.image.lower().endswith(('.png')):
        #     breakpoint
        #     messages.warning(request , 'Wrong Image Format')
        # else:
        broiler_image = image_form.save()

        
        interpreter = tf.lite.Interpreter(model_path="/home/takudzwa/Documents/Projects/broiler_disease/final_year_broiler/fomo_model.tflite")
        interpreter.allocate_tensors()
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()

        class_names = ['cocci', 'health', 'ncd','others'] 
        
        input_shape = (input_details[0]['shape'][1], input_details[0]['shape'][2])
        processed_image = preprocess_image(broiler_image.image, input_shape)

        if processed_image is not None:
            interpreter.set_tensor(input_details[0]['index'], processed_image)
            interpreter.invoke()
            output_data = interpreter.get_tensor(output_details[0]['index'])

            predicted_class_index = np.argmax(output_data) 
            predicted_probability = float(output_data[0][predicted_class_index])

            health_status = class_names[predicted_class_index]

            save = False

            if health_status == 'health':
                is_health = True
                diagnosis = "Broilers appear healthy"
                save = True
            elif health_status == 'others':
                print('here')
                is_health = False
                save = False
            else:
                save = True
                try:
                    fowlrun_temperature, fowlrun_humidity, fowlrun, timestamp = update_function()
                    time_difference = (timezone.now() - timestamp).total_seconds() 
                    if time_difference < 600:
                        high_risk_environment = (fowlrun_temperature > 20 and 
                                               fowlrun_temperature < 30 and 
                                               fowlrun_humidity > 60)
                        
                        if high_risk_environment:
                            diagnosis = "High coccidiosis risk confirmed and environmental conditions are conducive for the disease"
                        else:
                            diagnosis = "Possible coccidiosis detected, but environmental conditions good"
                    else:
                        diagnosis = "High coccidiosis risk confirmed but environmental conditions not taken into account"

                except FowlRunConditions.DoesNotExist:
                    diagnosis = "Possible coccidiosis detected, but no environmental data available"
                
                is_health = False
            if save:
                alerts = Alerts(
                    detection=health_status, 
                    is_health=is_health,
                    probability=predicted_probability,
                    detection_method='image',
                    diagnosis=diagnosis
                )
                alerts.save()
            broiler_image.health_status = health_status
            broiler_image.save()
            
            print(health_status,'health')
            if is_health:
                messages.success(request, f'Broiler Health status: {health_status}, Probability: {predicted_probability:.4f}')
            elif health_status == "others":
                 messages.warning(request, f'The Selected Image is Not A broiler Droppings  Probability: {predicted_probability:.4f}')
            else:
                messages.warning(request, f'Broiler Health status: {health_status}, Probability: {predicted_probability:.4f}')
            
            return True
        else:
            messages.warning(request, 'Wrong Image The Image type is not Supported')
            return False
            
    except Exception as e:
        messages.error(request, f'Error processing image: {str(e)}')
        return False


# AUDIO

def extract_audio_features(waveform, sample_rate):
    yamnet_model = hub.load('https://tfhub.dev/google/yamnet/1')
    scores, embeddings, spectrogram = yamnet_model(waveform)
    return embeddings.numpy().mean(axis=0)


def generate_diagnosis(health_status, probability):
    """Generate diagnosis message based on health status and environment"""
    try:
        env_data = FowlRunConditions.objects.latest('timestamp')
        time_diff = (timezone.now() - env_data.timestamp).total_seconds()
        
        if health_status != 'healthy':
            if time_diff < 600:  
                risk_conditions = (
                    env_data.temperature > 20 and 
                    env_data.temperature < 30 and 
                    env_data.humidity > 60
                )
                
                if risk_conditions:
                    return "Disease confirmed with high-risk environmental conditions"
                else:
                    return "Disease detected but environmental conditions are good"
            else:
                return "Disease detected (environmental data outdated)"
        else:
            return "Healthy broiler sounds detected"
            
    except FowlRunConditions.DoesNotExist:
        return "No environmental data available" if health_status != 'healthy' else "Healthy broiler sounds detected"


def process_audio_upload(request, audio_form):
    try:
        audio_feed = audio_form.save(commit=False)
        
        audio_path = audio_feed.audio_file.path
        waveform, sample_rate = librosa.load(audio_path, sr=16000, mono=True)
        duration = librosa.get_duration(y=waveform, sr=sample_rate)
        
        audio_feed.duration = duration
        audio_feed.sample_rate = sample_rate
        
      
        interpreter = tf.lite.Interpreter(model_path='/home/takudzwa/Documents/Projects/broiler_disease/final_year_broiler/broiler_disease_audio.tflite')
        interpreter.allocate_tensors()
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        
        features = extract_audio_features(waveform, sample_rate)
        
        input_data = np.expand_dims(features, axis=0).astype(np.float32)
        interpreter.set_tensor(input_details[0]['index'], input_data)
        interpreter.invoke()
        output_data = interpreter.get_tensor(output_details[0]['index'])
        
        class_names = ['healthy','ncd', 'noise' ]  
        predicted_class_index = np.argmax(output_data)
        predicted_probability = float(output_data[0][predicted_class_index])
        health_status = class_names[predicted_class_index]
        
        audio_feed.analysis_result = {
            'status': health_status,
            'confidence': predicted_probability,
            'class_distribution': output_data[0].tolist()
        }
        audio_feed.save()
        
        diagnosis = generate_diagnosis(health_status, predicted_probability)
        
        alert = Alerts(
            detection=health_status,
            is_health=(health_status == 'healthy'),
            probability=predicted_probability,
            detection_method='audio',
            diagnosis=diagnosis,
          
        )
        alert.save()
        
        if health_status == 'healthy':
            messages.success(request, f'Audio analysis: Healthy (confidence: {predicted_probability:.2%})')
        else:
            messages.warning(request, f'Audio analysis: {health_status} detected (confidence: {predicted_probability:.2%})')
        
        return True
        
    except Exception as e:
        messages.error(request, f'Error processing audio: {str(e)}')
        return False

def farmer_hub(request):
    image_form = BroilerImageForm()
    audio_form = BroilerAudioForm()
    
    if request.method == 'POST':
        form_type = request.POST.get('form_type')
        
        if form_type == 'audio':
            
            audio_form = BroilerAudioForm(request.POST, request.FILES)
            if audio_form.is_valid():
                if process_audio_upload(request, audio_form):
                    return redirect('dashboard')
                else:
                    return redirect('farmer_hub')
        
        elif form_type == 'image':
            image_form = BroilerImageForm(request.POST, request.FILES)
            if image_form.is_valid():
                # broiler_image = image_form.save(commit=False)
                # if not broiler_image.image.lower().endswith(('.png')):
                #     print('in this form')
                if process_image_upload(request, image_form):
                    return redirect('dashboard')
                else:
                    print('actually')
                    messages.warning(request, 'Image Not Supported')
                    return redirect('farmer_hub')
        
        else:
            messages.warning(request, 'Data not Supported')
            return redirect('farmer_hub')
    
    return render(request, 'add.html', {
        'image_form': image_form,
        'audio_form': audio_form
    })