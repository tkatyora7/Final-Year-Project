from django.urls import path
from . import views


urlpatterns=[
    path('',views.sign_in , name='login'),
    path('logout',views.signout , name='logout'),
    path('dashboard',views.dashboard , name='dashboard'),
    path('recommendations',views.recommendations , name='recommendations'),
    path('alerts',views.alerts , name='alerts'),
    path('camera-feed',views.camera_feed , name='camera_feed'),
    path('auido-feed',views.audio_feed , name='audio_feed'),
    path('add/', views.farmer_hub, name='farmer_hub'),
    
]
