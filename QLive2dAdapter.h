#pragma once

/*
* 作者：星辉清梦
* 作用：适配器
* 时间：乙巳己卯丁丑戊申
* 适配器：用于管理Live2d SDK，兼容String
* 触摸适配器：复制粘贴官方代码，就换了个Qt风格的名
* 内存适配器：复制粘贴官方代码，就换了个Qt风格的名
*/

#ifndef QLIVE2DADAPTER
#define QLIVE2DADAPTER
#include <array>
#include <string>
#include <fstream>

#include <QString>
#include <QByteArray>

#include <Math/CubismViewMatrix.hpp>
#include <Model/CubismUserModel.hpp>

//适配器
class QLive2dAdapter
{
public:
    static bool Good();     //判断SDK状态
    static void Enable();   //启用SDK
    static void Disable();  //关闭SDK

    static QString StdStringToQString(const std::string& string);
    static std::string QStringToStdString(const QString& string);
    static QString CsmStringToQString(const Csm::csmString& string);
    static std::string CsmStringToStdString(const Csm::csmString& string);
    static Csm::csmString QStringToCsmString(const QString& string);
    static Csm::csmString StdStringToCsmString(const std::string& string);

    static QByteArray LoadFileAsBytes(const QString& filePath);
    static const Csm::csmByte* toCsmByte(const QByteArray& arrays);
    static const Csm::csmSizeInt toCsmSize(const QByteArray& arrays);
};

//触摸适配器
class QLive2dTouch
{
public:
    QLive2dTouch();

    float GetCenterX() const { return _lastX; }
    float GetCenterY() const { return _lastY; }
    float GetDeltaX() const { return _deltaX; }
    float GetDeltaY() const { return _deltaY; }
    float GetStartX() const { return _startX; }
    float GetStartY() const { return _startY; }
    float GetScale() const { return _scale; }
    float GetX() const { return _lastX; }
    float GetY() const { return _lastY; }
    float GetX1() const { return _lastX1; }
    float GetY1() const { return _lastY1; }
    float GetX2() const { return _lastX2; }
    float GetY2() const { return _lastY2; }
    bool IsSingleTouch() const { return _touchSingle; }
    bool IsFlickAvailable() const { return _flipAvailable; }
    void DisableFlick() { _flipAvailable = false; }

    /*
    * @brief タッチ開始時イベント
    *
    * @param[in] deviceY    タッチした画面のyの値
    * @param[in] deviceX    タッチした画面のxの値
    */
    void TouchesBegan(float deviceX, float deviceY);

    /*
    * @brief ドラッグ時のイベント
    *
    * @param[in] deviceX    タッチした画面のyの値
    * @param[in] deviceY    タッチした画面のxの値
    */
    void TouchesMoved(float deviceX, float deviceY);

    /*
    * @brief ドラッグ時のイベント
    *
    * @param[in] deviceX1   1つめのタッチした画面のxの値
    * @param[in] deviceY1   1つめのタッチした画面のyの値
    * @param[in] deviceX2   2つめのタッチした画面のxの値
    * @param[in] deviceY2   2つめのタッチした画面のyの値
    */
    void TouchesMoved(float deviceX1, float deviceY1, float deviceX2, float deviceY2);

    /*
    * @brief フリックの距離測定
    *
    * @return フリック距離
    */
    float GetFlickDistance() const;

private:
    /*
    * @brief 点1から点2への距離を求める
    *
    * @param[in] x1 1つめのタッチした画面のxの値
    * @param[in] y1 1つめのタッチした画面のyの値
    * @param[in] x2 2つめのタッチした画面のxの値
    * @param[in] y2 2つめのタッチした画面のyの値
    * @return   2点の距離
    */
    float CalculateDistance(float x1, float y1, float x2, float y2) const;

    /*
    * 二つの値から、移動量を求める。
    * 違う方向の場合は移動量０。同じ方向の場合は、絶対値が小さい方の値を参照する
    *
    * @param[in] v1    1つめの移動量
    * @param[in] v2    2つめの移動量
    *
    * @return   小さい方の移動量
    */
    float CalculateMovingAmount(float v1, float v2);

    float _startY = 0.f;              // タッチを開始した時のxの値
    float _startX = 0.f;              // タッチを開始した時のyの値
    float _lastX = 0.f;               // シングルタッチ時のxの値
    float _lastY = 0.f;               // シングルタッチ時のyの値
    float _lastX1 = 0.f;              // ダブルタッチ時の一つ目のxの値
    float _lastY1 = 0.f;              // ダブルタッチ時の一つ目のyの値
    float _lastX2 = 0.f;              // ダブルタッチ時の二つ目のxの値
    float _lastY2 = 0.f;              // ダブルタッチ時の二つ目のyの値
    float _lastTouchDistance = 0.f;   // 2本以上でタッチしたときの指の距離
    float _deltaX = 0.f;              // 前回の値から今回の値へのxの移動距離。
    float _deltaY = 0.f;              // 前回の値から今回の値へのyの移動距離。
    float _scale = 1.f;               // このフレームで掛け合わせる拡大率。拡大操作中以外は1。
    bool _touchSingle = false;          // シングルタッチ時はtrue
    bool _flipAvailable = false;        // フリップが有効かどうか

};

//内存适配器
class QLive2dAllocator : public Csm::ICubismAllocator
{
	virtual void* Allocate(const Csm::csmSizeType  size);

	virtual void Deallocate(void* memory);

	virtual void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment);

	virtual void DeallocateAligned(void* alignedMemory);
};

#endif