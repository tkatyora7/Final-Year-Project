from django.contrib import admin
from .models import  BroilerImage ,Alerts ,AudioFeed , SoundAnalysis , Users



admin.site.register(Alerts)
admin.site.register(Users)
admin.site.register(SoundAnalysis)

@admin.register(BroilerImage)
class BroilerImageAdmin(admin.ModelAdmin):
    list_display = ('image', 'timestamp', 'fowlrun')
    list_filter = ('fowlrun',)

@admin.register(AudioFeed)
class AudioFeedAdmin(admin.ModelAdmin):
    list_display = ('audio_file', 'timestamp', )
