from django.db import models
from django.contrib.auth.models import AbstractUser
from phonenumber_field.modelfields import PhoneNumberField
from django.core.validators import MinValueValidator, MaxValueValidator
from django.db import models




FOWLRUN = [
    ("1","Fowl Run 1"),
    ("2","Fowl Run 2")
]


class Users(AbstractUser):
    email = models.EmailField(unique=True,null=True)
    username = models.CharField(max_length=255,unique=True,null=True)
    full_name = models.CharField(max_length=255,null=True)
    phone_number = PhoneNumberField(null=True)
    groups = models.ManyToManyField('auth.Group', related_name='Users_user_set', blank=True)
    user_permissions = models.ManyToManyField('auth.Permission', related_name='Users_user_set', blank=True)

    USERNAME_FIELD = 'username'
    REQUIRED_FIELDS =['email']

    def __str__(self):
        return self.email
    

class BroilerImage(models.Model):
    image = models.ImageField(upload_to='broiler_imagies')
    timestamp = models.DateTimeField(auto_now_add=True)
    health_status = models.CharField(max_length=255,null=True)
    fowlrun = models.CharField(max_length=255,choices=FOWLRUN,default='1')
    

    @property
    def url(self):
        return self.image.url
    
    class Meta:
        ordering = ['-timestamp']



class SoundAnalysis(models.Model):
    sound_count = models.IntegerField()
    sound_status = models.CharField(max_length=20)  
    risk_level = models.CharField(max_length=20)  
    prolong_silence = models.BooleanField(default=False) 
    timestamp = models.DateTimeField(auto_now_add=True)
    temperature = models.FloatField(validators=[MinValueValidator(10), MaxValueValidator(40)],null=True)
    humidity = models.FloatField(validators=[MinValueValidator(30), MaxValueValidator(90)],null=True)

    def __str__(self):
        return f"SoundAnalysis @ {self.timestamp} - Risk: {self.risk_level}"


class AudioFeed(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True,null=True)
    audio_file = models.FileField(upload_to='audio/',null=True)
    duration = models.FloatField(help_text="Duration in seconds",null=True)
    sample_rate = models.IntegerField(default=8000,null=True)
    analysis_result = models.JSONField(null=True, blank=True)
    temperature = models.FloatField(validators=[MinValueValidator(10), MaxValueValidator(40)],null=True)
    humidity = models.FloatField(validators=[MinValueValidator(30), MaxValueValidator(90)],null=True)
    fowlrun = models.CharField(max_length=255,choices=FOWLRUN,default='1')
    
    class Meta:
        ordering = ['timestamp']
    
    def get_absolute_url(self):
        return self.audio_file.url
        


class FowlRunConditions(models.Model):
    temperature = models.FloatField(validators=[MinValueValidator(10), MaxValueValidator(40)],null=True)
    humidity = models.FloatField(validators=[MinValueValidator(30), MaxValueValidator(90)],null=True)
    timestamp = models.DateTimeField(auto_now_add=True)
    fowlrun = models.CharField(max_length=50,null=True,default='1',choices=FOWLRUN)
    

    class Meta:
        ordering = ['-timestamp']



class Alerts(models.Model):
    DETECTION=[('cocci','cocci'),
               ('ncd','ncd'),
               ('health','health'),

               ]
    DETECTION_METHOD=[('sound','image_sound'),
               ('image','image_dropings'),
                   ('all','both'),
               ]
    timestamp = models.DateTimeField(auto_now_add=True)
    fowlrun = models.CharField(max_length=255,choices=FOWLRUN,default='1')
    is_health= models.BooleanField(default=True,null=True)
    detection = models.CharField(max_length=255,choices=DETECTION,null=True)
    probability = models.DecimalField(max_digits=7, decimal_places=5, null=True, blank=True)
    detection_method = models.CharField(max_length=255,choices=DETECTION_METHOD,null=True)
    diagnosis = models.CharField(max_length=255,null=True)

   
    
    class Meta:
        ordering = ['-timestamp']
        










        

    
