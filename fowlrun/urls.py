from django.urls import path
from .views import fowlrun_conditions , fowlrun_images ,upload_audio_analysis
from . import views


urlpatterns=[
    path('fowlrun-conditions/',fowlrun_conditions),
    path('fowlrun-image/',fowlrun_images),
    path('audio/analysis/', upload_audio_analysis),
    path('api/device_ping/', views.device_ping, name='device_ping'),
]
