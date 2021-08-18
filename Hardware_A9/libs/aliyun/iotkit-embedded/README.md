# Introduction to Link Kit C-SDK

After the device manufacturer integrates the `Link Kit C-SDK` on the device, the device can be safely connected to the Alibaba Cloud IoT platform, so that the device can be managed by the Alibaba Cloud IoT platform

The device needs to support the TCP/IP protocol stack to integrate the SDK, non-IP devices such as zigbee/433/KNX need to be connected to the Alibaba Cloud IoT platform through the gateway device, and the gateway device needs to integrate the Link Kit SDK

# Quick start

Users can use [Quickly experience Link Kit SDK in Ubuntu](https://help.aliyun.com/document_detail/96624.html) to experience how to connect devices to the Alibaba Cloud IoT platform and how to send device data to Platform/and how to receive data from IoT platform

# Porting instructions

The Link Kit SDK is a code written in C that has nothing to do with the OS/hardware platform. It defines the HAL layer to interface with hardware-related functions. Therefore, users need to implement related HAL functions when using the Link Kit SDK.

At present, the Link Kit SDK has realized the realization of HAL on Linux/Windows/AliOS, and it has also been adapted to some common OS or modules. You can [visit here](https://help.aliyun.com/ document_detail/97557.html) See how to compile and integrate the SDK on the corresponding platform

# Programming documentation

The SDK provides a series of programming documents to describe how to use the software functions provided by the SDK, please [visit here](https://help.aliyun.com/document_detail/96627.html) for understanding