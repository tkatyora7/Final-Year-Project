from django.db import models
from django.utils import timezone
import datetime
# import tensorflow_hub as hub
# import tensorflow as tf

# # Create your models here.


class DeviceStatus(models.Model):
    name = models.CharField(max_length=50)
    last_ping = models.DateTimeField(null=True, blank=True)
    is_online = models.BooleanField(default=False)

    def update_status(self):
        """Update online status based on last ping time"""
        if self.last_ping:
          
            self.is_online = (timezone.now() - self.last_ping).total_seconds() < 600
            self.save()
        else:
            self.is_online = False
            self.save()
        return self.is_online

    def __str__(self):
        return f"{self.name} - {'Online' if self.is_online else 'Offline'}"