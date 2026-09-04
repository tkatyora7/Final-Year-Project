from rest_framework import serializers 
from main_application.models import FowlRunConditions ,BroilerImage
from django.conf import settings


class  FowlRunConditionSerializer(serializers.ModelSerializer):
   
    class Meta:
        model = FowlRunConditions
        fields = ['temperature','humidity']


class  ImageSerializer(serializers.ModelSerializer):
   
    class Meta:
        model = BroilerImage
        fields = ['image','fowlrun']