from django import forms
from .models import BroilerImage , AudioFeed

class BroilerImageForm(forms.ModelForm):
    class Meta:
        model = BroilerImage
        fields = ['image', 'fowlrun']
        
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.fields['image'].widget.attrs.update({'class': 'block w-full text-sm text-gray-900 border border-gray-300 rounded-lg cursor-pointer bg-gray-50 focus:outline-none'})
        self.fields['fowlrun'].widget.attrs.update({'class': 'bg-gray-50 border border-gray-300 text-gray-900 text-sm rounded-lg focus:ring-blue-500 focus:border-blue-500 block w-full p-2.5'})


class BroilerAudioForm(forms.ModelForm):
  
    
    class Meta:
        model = AudioFeed
        fields = ['audio_file', 'fowlrun']
        widgets = {
            'audio_file': forms.FileInput(attrs={'accept': 'audio/*'}),
        }