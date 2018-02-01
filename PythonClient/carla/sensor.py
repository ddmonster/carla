# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA sensors."""


import os
import numpy as np
import json


# ==============================================================================
# -- Sensor --------------------------------------------------------------------
# ==============================================================================


class Sensor(object):
    """
    Base class for sensor descriptions. Used to add sensors to CarlaSettings.
    """

    def set(self, **kwargs):
        for key, value in kwargs.items():
            if not hasattr(self, key):
                raise ValueError('CarlaSettings.Camera: no key named %r' % key)
            setattr(self, key, value)


class Camera(Sensor):
    """
    Camera description. This class can be added to a CarlaSettings object to add
    a camera to the player vehicle.
    """

    def __init__(self, name, **kwargs):
        self.CameraName = name
        self.PostProcessing = 'SceneFinal'
        self.ImageSizeX = 800
        self.ImageSizeY = 600
        self.CameraFOV = 90
        self.CameraPositionX = 140
        self.CameraPositionY = 0
        self.CameraPositionZ = 140
        self.CameraRotationPitch = 0
        self.CameraRotationRoll = 0
        self.CameraRotationYaw = 0
        self.set(**kwargs)

    def set_image_size(self, pixels_x, pixels_y):
        self.ImageSizeX = pixels_x
        self.ImageSizeY = pixels_y

    def set_position(self, x, y, z):
        self.CameraPositionX = x
        self.CameraPositionY = y
        self.CameraPositionZ = z

    def set_rotation(self, pitch, roll, yaw):
        self.CameraRotationPitch = pitch
        self.CameraRotationRoll = roll
        self.CameraRotationYaw = yaw


class Lidar(Sensor):
    """
    Lidar description. This class can be added to a CarlaSettings object to add
    a Lidar to the player vehicle.
    """

    def __init__(self, name, **kwargs):
        self.LidarName = name
        # Number of lasers
        self.Channels = 32
        # Measure distance
        self.Range = 5000
        # Points generated by all lasers per second
        self.PointsPerSecond = 100000
        # Lidar rotation frequency
        self.RotationFrequency = 10
        # Upper laser angle, counts from horizontal,
        # positive values means above horizontal line
        self.UpperFovLimit = 10
        # Lower laser angle, counts from horizontal,
        # negative values means under horizontal line
        self.LowerFovLimit = -30
        # wether to show debug points of laser hits in simulator
        self.ShowDebugPoints = False
        # Position relative to the player.
        self.LidarPositionX = 0
        self.LidarPositionY = 0
        self.LidarPositionZ = 250
        # Rotation relative to the player.
        self.LidarRotationPitch = 0
        self.LidarRotationRoll = 0
        self.LidarRotationYaw = 0
        self.set(**kwargs)

    def set_position(self, x, y, z):
        self.LidarPositionX = x
        self.LidarPositionY = y
        self.LidarPositionZ = z

    def set_rotation(self, pitch, roll, yaw):
        self.LidarRotationPitch = pitch
        self.LidarRotationRoll = roll
        self.LidarRotationYaw = yaw


# ==============================================================================
# -- SensorData ----------------------------------------------------------------
# ==============================================================================


class SensorData(object):
    """Base class for sensor data returned from the server."""
    pass


class Image(SensorData):
    """Data generated by a Camera."""

    def __init__(self, width, height, image_type, raw_data):
        assert len(raw_data) == 4 * width * height
        self.width = width
        self.height = height
        self.type = image_type
        self.raw_data = raw_data
        self._converted_data = None

    @property
    def data(self):
        """
        Lazy initialization for data property, stores converted data in its
        default format.
        """
        if self._converted_data is None:
            from . import image_converter

            if self.type == 'Depth':
                self._converted_data = image_converter.depth_to_array(self)
            elif self.type == 'SemanticSegmentation':
                self._converted_data = image_converter.labels_to_array(self)
            else:
                self._converted_data = image_converter.to_rgb_array(self)
        return self._converted_data

    def save_to_disk(self, filename):
        """Save this image to disk (requires PIL installed)."""
        try:
            from PIL import Image as PImage
        except ImportError:
            raise RuntimeError('cannot import PIL, make sure pillow package is installed')

        image = PImage.frombytes(
            mode='RGBA',
            size=(self.width, self.height),
            data=self.raw_data,
            decoder_name='raw')
        b, g, r, _ = image.split()
        image = PImage.merge("RGB", (r, g, b))

        folder = os.path.dirname(filename)
        if not os.path.isdir(folder):
            os.makedirs(folder)
        image.save(filename)


class LidarMeasurement(SensorData):
    """Data generated by a Lidar."""

    def __init__(self, horizontal_angle, channels_count, lidar_type, raw_data):
        self.horizontal_angle = horizontal_angle
        self.channels_count = channels_count
        self.type = lidar_type
        self._converted_data = None

        points_count_by_channel_size = channels_count * 4
        points_count_by_channel_bytes = raw_data[4*4:4*4 + points_count_by_channel_size]
        self.points_count_by_channel = np.frombuffer(points_count_by_channel_bytes, dtype=np.dtype('uint32'))

        self.points_size = int(np.sum(self.points_count_by_channel) * 3 * 8)  # three floats X, Y, Z
        begin = 4*4 + points_count_by_channel_size  # 4*4 is horizontal_angle, type, channels_count
        end = begin + self.points_size
        self.points_data = raw_data[begin:end]

        self._data_size = 4*4 + points_count_by_channel_size + self.points_size

    @property
    def size_in_bytes(self):
        return self._data_size

    @property
    def data(self):
        """
        Lazy initialization for data property, stores converted data in its
        default format.
        """
        if self._converted_data is None:

            points_in_one_channel = self.points_count_by_channel[0]
            points = np.frombuffer(self.points_data[:self.points_size], dtype='float')
            points = np.reshape(points, (self.channels_count, points_in_one_channel, 3))

            self._converted_data = {
                'horizontal_angle' : self.horizontal_angle,
                'channels_count' : self.channels_count,
                'points_count_by_channel' : self.points_count_by_channel,
                'points' : points
            }

        return self._converted_data

    def save_to_disk(self, filename):
        """Save lidar measurements to disk"""
        folder = os.path.dirname(filename)
        if not os.path.isdir(folder):
            os.makedirs(folder)
        with open(filename, 'wt') as f:
            f.write(json.dumps({
                'horizontal_angle' : self.horizontal_angle,
                'channels_count' : self.channels_count,
                'points_count_by_channel' : self.points_count_by_channel.tolist(),
                'points' : self.data['points'].tolist()
            }))
