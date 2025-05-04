#include "QLive2dAdapter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

std::size_t isEnable = 0;
QLive2dAllocator _cubismAllocator;              ///< Cubism SDK Allocator
bool QLive2dAdapter::Good()
{
    return (isEnable != 0);
}
void QLive2dAdapter::Enable()
{
    if (isEnable == 0)
    {
        Csm::CubismFramework::StartUp(&_cubismAllocator);
        Csm::CubismFramework::Initialize();// 启用 SDK（程序进入时调用）
    }
    isEnable++;
}
void QLive2dAdapter::Disable()
{
    if (isEnable > 0) isEnable--;
    if (isEnable == 0) Csm::CubismFramework::Dispose(); // 关闭 SDK（程序退出前调用）
}

QString QLive2dAdapter::StdStringToQString(const std::string& string)
{
    QString res = QString::fromStdString(string);
    return res;
}

std::string QLive2dAdapter::QStringToStdString(const QString& string)
{
    std::string res = string.toStdString();
    return res;
}

QString QLive2dAdapter::CsmStringToQString(const Csm::csmString& string)
{
    std::string stdString = string.GetRawString();
    QString res = QString::fromStdString(stdString);
    return res;
}

std::string QLive2dAdapter::CsmStringToStdString(const Csm::csmString& string)
{
    std::string res = string.GetRawString();
    return res;
}

Csm::csmString QLive2dAdapter::QStringToCsmString(const QString& string)
{
    std::string stdString = string.toStdString();
    Csm::csmString res = stdString.data();
    return res;
}

Csm::csmString QLive2dAdapter::StdStringToCsmString(const std::string& string)
{
    Csm::csmString res = string.data();
    return res;
}

QByteArray QLive2dAdapter::LoadFileAsBytes(const QString& filePath)
{
    QByteArray readFileBytes;
    std::wfstream file(filePath.toStdWString(), std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        if (fileSize != std::streampos(-1))
        {
            file.seekg(0, 0);
            unsigned int dataSize = fileSize;
            readFileBytes.resize(dataSize);
            std::wfilebuf* fileBuf = file.rdbuf();
            for (unsigned int i = 0; i < dataSize; i++)
            {
                readFileBytes[i] = fileBuf->sbumpc();
            }
        }
        file.close();
    }
    return readFileBytes;
}

const Csm::csmByte* QLive2dAdapter::toCsmByte(const QByteArray& arrays)
{
    return (const Csm::csmByte*)(arrays.data());
}

const Csm::csmSizeInt QLive2dAdapter::toCsmSize(const QByteArray& arrays)
{
    return arrays.size();
}



void* QLive2dAllocator::Allocate(const Csm::csmSizeType size)
{
	return malloc(size);
}

void QLive2dAllocator::Deallocate(void* memory)
{
	free(memory);
}

void* QLive2dAllocator::AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment)
{
	size_t offset, shift, alignedAddress;
	void* allocation;
	void** preamble;

	offset = alignment - 1 + sizeof(void*);

	allocation = Allocate(size + static_cast<Csm::csmUint32>(offset));

	alignedAddress = reinterpret_cast<size_t>(allocation) + sizeof(void*);

	shift = alignedAddress % alignment;

	if (shift)
	{
		alignedAddress += (alignment - shift);
	}

	preamble = reinterpret_cast<void**>(alignedAddress);
	preamble[-1] = allocation;

	return reinterpret_cast<void*>(alignedAddress);
}

void QLive2dAllocator::DeallocateAligned(void* alignedMemory)
{
	void** preamble = static_cast<void**>(alignedMemory);

	Deallocate(preamble[-1]);
}



QLive2dTouch::QLive2dTouch()
    : _startY(0.0f)
    , _startX(0.0f)
    , _lastX(0.0f)
    , _lastY(0.0f)
    , _lastX1(0.0f)
    , _lastY1(0.0f)
    , _lastX2(0.0f)
    , _lastY2(0.0f)
    , _lastTouchDistance(0.0f)
    , _deltaX(0.0f)
    , _deltaY(0.0f)
    , _scale(1.0f)
    , _touchSingle(false)
    , _flipAvailable(false)
{ }

void QLive2dTouch::TouchesBegan(float deviceX, float deviceY)
{
    _lastX = deviceX;
    _lastY = deviceY;
    _startX = deviceX;
    _startY = deviceY;
    _lastTouchDistance = -1.0f;
    _flipAvailable = true;
    _touchSingle = true;
}

void QLive2dTouch::TouchesMoved(float deviceX, float deviceY)
{
    _lastX = deviceX;
    _lastY = deviceY;
    _lastTouchDistance = -1.0f;
    _touchSingle = true;
}

void QLive2dTouch::TouchesMoved(float deviceX1, float deviceY1, float deviceX2, float deviceY2)
{
    float distance = CalculateDistance(deviceX1, deviceY1, deviceX2, deviceY2);
    float centerX = (deviceX1 + deviceX2) * 0.5f;
    float centerY = (deviceY1 + deviceY2) * 0.5f;

    if (_lastTouchDistance > 0.0f)
    {
        _scale = powf(distance / _lastTouchDistance, 0.75f);
        _deltaX = CalculateMovingAmount(deviceX1 - _lastX1, deviceX2 - _lastX2);
        _deltaY = CalculateMovingAmount(deviceY1 - _lastY1, deviceY2 - _lastY2);
    }
    else
    {
        _scale = 1.0f;
        _deltaX = 0.0f;
        _deltaY = 0.0f;
    }

    _lastX = centerX;
    _lastY = centerY;
    _lastX1 = deviceX1;
    _lastY1 = deviceY1;
    _lastX2 = deviceX2;
    _lastY2 = deviceY2;
    _lastTouchDistance = distance;
    _touchSingle = false;
}

float QLive2dTouch::GetFlickDistance() const
{
    return CalculateDistance(_startX, _startY, _lastX, _lastY);
}

float QLive2dTouch::CalculateDistance(float x1, float y1, float x2, float y2) const
{
    return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

float QLive2dTouch::CalculateMovingAmount(float v1, float v2)
{
    if ((v1 > 0.0f) != (v2 > 0.0f))
    {
        return 0.0f;
    }

    float sign = v1 > 0.0f ? 1.0f : -1.0f;
    float absoluteValue1 = fabsf(v1);
    float absoluteValue2 = fabsf(v2);
    return sign * ((absoluteValue1 < absoluteValue2) ? absoluteValue1 : absoluteValue2);
}