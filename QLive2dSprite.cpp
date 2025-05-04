#define GLEW_STATIC
#include <GL/glew.h>
#include <QAudioProbe>
#include <QMediaPlayer>

#include "QLive2dSprite.h"

#include <CubismModelSettingJson.hpp>
#include <CubismDefaultParameterId.hpp>

#include <Id/CubismIdManager.hpp>
#include <Math/CubismViewMatrix.hpp>
#include <Utils/CubismString.hpp>
#include <Motion/CubismMotion.hpp>
#include <Physics/CubismPhysics.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>


QVoice::QVoice()
	: m_SoundPath("")
{
}

QVoice::~QVoice()
{
	m_SoundPath = "";
}

bool QVoice::LoadSound(const QString& SoundPath)
{
	if (QFile::exists(SoundPath)) 
	{ 
		m_SoundPath = SoundPath;
		return true;
	}
	else
	{
		m_SoundPath = "";
	}
	return false;
}


QShader::QShader(QOpenGLContext* glContext)
	: m_programId(0), m_glContext(glContext)
{
	if (!m_glContext) m_glContext = QOpenGLContext::currentContext();
}

QShader::~QShader()
{
	if (m_programId != 0 && m_glContext) m_glContext->functions()->glDeleteShader(m_programId);
}

bool QShader::LoadShader(const QString& vertPath, const QString& fragPath)
{
	if (m_glContext)
	{
		auto glFunction = m_glContext->functions();
		if (m_programId != 0) glFunction->glDeleteShader(m_programId);
		GLuint vertexShaderId = _CompileShader(vertPath, GL_VERTEX_SHADER);
		GLuint fragmentShaderId = _CompileShader(fragPath, GL_FRAGMENT_SHADER);

		m_programId = 0;
		if (vertexShaderId && fragmentShaderId)
		{
			auto programId = glFunction->glCreateProgram();
			glFunction->glAttachShader(programId, vertexShaderId);
			glFunction->glAttachShader(programId, fragmentShaderId);
			glFunction->glLinkProgram(programId);
			glFunction->glUseProgram(programId);
			m_programId = programId;
		}

		if (vertexShaderId) glFunction->glDeleteShader(vertexShaderId);
		if (fragmentShaderId) glFunction->glDeleteShader(fragmentShaderId);
	}

	return (m_programId != 0);
}

GLuint QShader::_CompileShader(const QString& shaderPath, GLenum shaderType)
{
	QFile shader(shaderPath);
	shader.open(QFile::ReadOnly);
	auto Data = shader.readAll();
	shader.close();

	const GLint glSize = Data.size();
	const char* glData = Data.data();

	auto glFunction = m_glContext->functions();
	GLuint shaderId = glFunction->glCreateShader(shaderType);
	glFunction->glShaderSource(shaderId, 1, &glData, &glSize);
	glFunction->glCompileShader(shaderId);

	GLint status;
	GLint length;
	QByteArray InfoLog;
	glFunction->glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		InfoLog.resize(length);
		glFunction->glGetShaderInfoLog(shaderId, length, &length, InfoLog.data());
	}

	glFunction->glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		glFunction->glDeleteShader(shaderId);
		shaderId = 0;
	}

	return shaderId;
}


QTexture::QTexture(QOpenGLContext* glContext)
	: m_programId(0), m_width(0), m_height(0), m_glContext(glContext)
{
	if (!m_glContext) m_glContext = QOpenGLContext::currentContext();
}

QTexture::~QTexture()
{
	if (m_programId != 0 && m_glContext) m_glContext->functions()->glDeleteTextures(1, &m_programId);
}

bool QTexture::LoadTextures(const QString& filePath)
{
	if (m_glContext)
	{
		auto glFunction = m_glContext->functions();
		if (m_programId != 0) glFunction->glDeleteTextures(1, &m_programId);

		m_programId = 0;
		QImage texture(filePath); // test image
		texture = texture.convertToFormat(QImage::Format_RGBA8888);
		if (!texture.isNull())
		{
			m_width = texture.width();
			m_height = texture.height();
			glFunction->glGenTextures(1, &m_programId);
			glFunction->glBindTexture(GL_TEXTURE_2D, m_programId);
			glFunction->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
			glFunction->glGenerateMipmap(GL_TEXTURE_2D);
			glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFunction->glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	return (m_programId != 0);
}


struct QLive2dSprite::_impClass
{
	//时间管理
	double _currFrame = 0.0;
	double _lastFrame = 0.0;
	double _deltaTime = 0.0;
	double _renderTimeSec = 0.f;
	QTimer* _printTimer = nullptr;
	//空间管理
	QLive2dTouch _touchManager;
	std::array<float, 4> _spriteColor = {};		//背景颜色
	Csm::CubismMatrix44 _spriteMatrix = {};		//模型变换
	Csm::CubismPriority _spritePriority = {};	//优先等级

	//固定数据
	const Csm::CubismId* _idParamHeadAngleX = nullptr; ///< パラメータID: ParamHeadAngleX
	const Csm::CubismId* _idParamHeadAngleY = nullptr; ///< パラメータID: ParamHeadAngleY
	const Csm::CubismId* _idParamHeadAngleZ = nullptr; ///< パラメータID: ParamHeadAngleZ
	const Csm::CubismId* _idParamBodyAngleX = nullptr; ///< パラメータID: ParamBodyAngleX
	const Csm::CubismId* _idParamEyeBallX = nullptr; ///< パラメータID: ParamEyeBallX
	const Csm::CubismId* _idParamEyeBallY = nullptr; ///< パラメータID: ParamEyeBallY
	//模型数据
	Csm::ICubismModelSetting* _Setting = nullptr; ///< モデルセッティング情報
	Csm::csmString _modelName = "";
	Csm::csmString _modelHome = "";
	QVector<QShader> _spriteShaders;
	QVector<QTexture> _spriteTextures;
	Csm::csmVector<Csm::csmRectF> _hitArea;
	Csm::csmVector<Csm::csmRectF> _useArea;
	Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds;
	Csm::csmVector<Csm::CubismIdHandle> _lipSyncsIds;
	Csm::csmMap<Csm::csmString, QVoice*>				_voices;
	Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>	_motions;
	Csm::csmMap<Csm::csmString, Csm::ACubismMotion*>	_expressions;

	QString currVoice = "";
	QString currMotion = "";
	QString currExpression = "";
	QAudioProbe* probe = nullptr;
	QMediaPlayer* player = nullptr;
	Csm::csmFloat32 voiceValue = 0.0f;

};

void QLive2dSprite::__connectBeganCallback(Csm::ACubismMotion* self)
{
	QLive2dSprite* model = (QLive2dSprite*)(self->GetBeganMotionCustomData());
	model->motionsBegan(self);
}

void QLive2dSprite::__connectFinishedCallback(Csm::ACubismMotion* self)
{
	QLive2dSprite* model = (QLive2dSprite*)(self->GetFinishedMotionCustomData());
	model->motionsFinished(self);
}

QLive2dSprite::QLive2dSprite(QWidget* parent)
	: QOpenGLWidget(parent), m_packet(new QLive2dSprite::_impClass())
{
	m_packet->probe = new QAudioProbe(this);
	m_packet->player = new QMediaPlayer(this);
	m_packet->probe->setSource(m_packet->player);

	connect(this, &QLive2dSprite::voicesChanged, [this](QString t) {this->m_packet->currVoice = t; });
	connect(this, &QLive2dSprite::motionsChanged, [this](QString t) {this->m_packet->currMotion = t; });
	connect(this, &QLive2dSprite::expressionsChanged, [this](QString t) {this->m_packet->currExpression = t; });

	connect(m_packet->probe, &QAudioProbe::audioBufferProbed, [this](const QAudioBuffer& buffer)
	{
		// 计算平均音量（绝对值）
		Csm::csmFloat32 rms = 0.0f;
		const qint16* pcmData = buffer.constData<qint16>();
		int samples = buffer.frameCount() * buffer.format().channelCount();
		for (int i = 0; i < samples; ++i)
		{ 
			const Csm::csmFloat32 pcm = pcmData[i] / 32768.0f;;
			rms += (pcm * pcm);
		}
		rms = samples > 0 ? std::sqrt(rms / samples) : 0.0f;
		m_packet->voiceValue = rms;
	});
	connect(m_packet->player, &QMediaPlayer::stateChanged, [this](QMediaPlayer::State state) {
		if (QMediaPlayer::StoppedState == state) this->m_packet->currVoice = "";
	});
	connect(this, &QLive2dSprite::motionsFinished, [this](Csm::ACubismMotion* self) {
		if (self) this->m_packet->currMotion = "";
	});
	
	//此处的执行顺序不可以改动，否则无法透明窗口
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	//setAttribute(Qt::WA_TransparentForMouseEvents); //鼠标穿透
	setAttribute(Qt::WA_TranslucentBackground); // 背景透明
	//setMouseTracking(true);
	show();
}

QLive2dSprite::~QLive2dSprite()
{
	ReleaseVoices();
	ReleaseMotions();
	ReleaseExpressions();
	delete m_packet->_printTimer;
	delete m_packet->_Setting;
	delete m_packet;
}


QString QLive2dSprite::spriteName() const
{
	return QLive2dAdapter::CsmStringToQString(m_packet->_modelName);
}

QString QLive2dSprite::spriteVoice() const
{
	return m_packet->currVoice;
}

QString QLive2dSprite::spriteMotion() const
{
	return m_packet->currMotion;
}

QString QLive2dSprite::spriteExpression() const
{
	return m_packet->currExpression;
}

QStringList QLive2dSprite::spriteVoices() const
{
	QStringList List;
	for (auto iter = m_packet->_voices.Begin();
		iter != m_packet->_voices.End(); ++iter)
	{
		List << QLive2dAdapter::CsmStringToQString(iter->First);
	}
	return List;
}

QStringList QLive2dSprite::spriteMotions() const
{
	QStringList List;
	for (auto iter = m_packet->_motions.Begin();
		iter != m_packet->_motions.End(); ++iter)
	{
		List << QLive2dAdapter::CsmStringToQString(iter->First);
	}
	return List;
}

QStringList QLive2dSprite::spriteExpressions() const
{
	QStringList List;
	for (auto iter = m_packet->_expressions.Begin();
		iter != m_packet->_expressions.End(); ++iter)
	{
		List << QLive2dAdapter::CsmStringToQString(iter->First);
	}
	return List;
}


void QLive2dSprite::setColor(float red, float green, float blue, float alph)
{
	m_packet->_spriteColor[0] = red;
	m_packet->_spriteColor[1] = green;
	m_packet->_spriteColor[2] = blue;
	m_packet->_spriteColor[3] = alph;
}

void QLive2dSprite::setMatrix(const Csm::CubismMatrix44& Matrix)
{
	m_packet->_spriteMatrix = Matrix;
}

void QLive2dSprite::setPriority(const Csm::CubismPriority& Priority)
{
	m_packet->_spritePriority = Priority;
}


bool QLive2dSprite::loadPath(const QString& Path)
{
	//文件不存在
	if (!QFile::exists(Path)) { emit loadModelFailed();; return false; }
	//窗口未初始化
	if (!m_packet->_printTimer) { emit loadModelFailed();; return false; }
	//已读取或正在读取
	if (_initialized || _updating) { emit loadModelFailed();; return false; }

	_updating = true;
	_initialized = false;
	QByteArray FileBuffer;
	this->makeCurrent();

	//数据目录解析
	QFileInfo PathInfo(Path);
	QString fileName = PathInfo.fileName();//文件名
	QString fileHome = PathInfo.absolutePath() + "/";//绝对路径
	m_packet->_modelName = QLive2dAdapter::QStringToCsmString(fileName);
	m_packet->_modelHome = QLive2dAdapter::QStringToCsmString(fileHome);

	//Cubism Infomation
	Csm::ICubismModelSetting* setting = m_packet->_Setting;
	FileBuffer = QLive2dAdapter::LoadFileAsBytes(fileHome + fileName);
	m_packet->_Setting = new Csm::CubismModelSettingJson(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer));
	if (setting) delete setting;

	//Cubism Model
	if (strcmp(m_packet->_Setting->GetModelFileName(), "") != 0)
	{
		Csm::csmString readPath = m_packet->_modelHome + m_packet->_Setting->GetModelFileName();
		FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
		LoadModel(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer), _mocConsistency);
	}

	CreateRenderer();
	//这个QShader看起来没啥用处
	m_packet->_spriteShaders.push_back(QShader(this->context()));
	m_packet->_spriteShaders.front().LoadShader("SampleShaders/VertSprite.vert", "SampleShaders/FragSprite.frag");

	//Expression
	if (m_packet->_Setting->GetExpressionCount() > 0)
	{
		const Csm::csmInt32 count = m_packet->_Setting->GetExpressionCount();
		for (Csm::csmInt32 i = 0; i < count; i++)
		{
			Csm::csmString name = m_packet->_Setting->GetExpressionName(i);
			Csm::csmString path = m_packet->_Setting->GetExpressionFileName(i);
			Csm::csmString readPath = m_packet->_modelHome + path;
			FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
			Csm::ACubismMotion* motion = LoadExpression(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer), name.GetRawString());

			if (motion)
			{
				motion->SetBeganMotionHandlerAndMotionCustomData(QLive2dSprite::__connectBeganCallback, this);
				motion->SetFinishedMotionHandlerAndMotionCustomData(QLive2dSprite::__connectFinishedCallback, this);
				if (m_packet->_expressions[name] != nullptr)
				{
					Csm::ACubismMotion::Delete(m_packet->_expressions[name]);
					m_packet->_expressions[name] = nullptr;
				}
				m_packet->_expressions[name] = motion;
			}
		}
	}

	//Textures
	if (m_packet->_Setting->GetTextureCount() > 0)
	{
		const Csm::csmInt32 count = m_packet->_Setting->GetTextureCount();
		m_packet->_spriteTextures.resize(count);
		for (Csm::csmInt32 i = 0; i < count; i++)
		{
			if (strcmp(m_packet->_Setting->GetTextureFileName(i), "") != 0)
			{
				Csm::csmString texturePath = m_packet->_modelHome + m_packet->_Setting->GetTextureFileName(i);
				m_packet->_spriteTextures[i] = QTexture(this->context());
				m_packet->_spriteTextures[i].LoadTextures(QLive2dAdapter::CsmStringToQString(texturePath));
				GetRenderer<Csm::Rendering::CubismRenderer_OpenGLES2>()->BindTexture(i, m_packet->_spriteTextures[i].Id());
			}
		}
	}

	//Motions
	for (Csm::csmInt32 i = 0; i < m_packet->_Setting->GetMotionGroupCount(); i++)
	{
		const Csm::csmChar* group = m_packet->_Setting->GetMotionGroupName(i);
		const Csm::csmInt32 count = m_packet->_Setting->GetMotionCount(group);
		for (Csm::csmInt32 i = 0; i < count; i++)
		{
			//ex) idle_0
			Csm::csmString name = Csm::Utils::CubismString::GetFormatedString("%s_%d", group, i);
			Csm::csmString path = m_packet->_Setting->GetMotionFileName(group, i);
			Csm::csmString voice = m_packet->_Setting->GetMotionSoundFileName(group, i);
			if (strcmp(voice.GetRawString(), "") != 0)
			{
				Csm::csmString readPath =  m_packet->_modelHome + voice;
				QVoice* sound = new QVoice();
				
				if (sound->LoadSound(QLive2dAdapter::CsmStringToQString(readPath)))
				{
					m_packet->_voices[name] = sound;
				}
				else
				{
					delete sound;
				}
			}

			Csm::csmString readPath = m_packet->_modelHome + path;
			FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
			Csm::CubismMotion* tmpMotion = static_cast<Csm::CubismMotion*>
				(
					LoadMotion(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer), name.GetRawString(), nullptr, nullptr, m_packet->_Setting, group, i)
				);
			if (tmpMotion)
			{
				tmpMotion->SetEffectIds(m_packet->_eyeBlinkIds, m_packet->_lipSyncsIds);
				tmpMotion->SetBeganMotionHandlerAndMotionCustomData(QLive2dSprite::__connectBeganCallback, this);
				tmpMotion->SetFinishedMotionHandlerAndMotionCustomData(QLive2dSprite::__connectFinishedCallback, this);
				if (m_packet->_motions[name]) { Csm::ACubismMotion::Delete(m_packet->_motions[name]); }
				m_packet->_motions[name] = tmpMotion;
			}
		}
	}

	//Physics
	if (strcmp(m_packet->_Setting->GetPhysicsFileName(), "") != 0)
	{
		Csm::csmString readPath = m_packet->_modelHome + m_packet->_Setting->GetPhysicsFileName();
		FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
		LoadPhysics(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer));
	}

	//Pose
	if (strcmp(m_packet->_Setting->GetPoseFileName(), "") != 0)
	{
		Csm::csmString readPath = m_packet->_modelHome + m_packet->_Setting->GetPoseFileName();
		FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
		LoadPose(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer));
	}

	//UserData
	if (strcmp(m_packet->_Setting->GetUserDataFile(), "") != 0)
	{
		Csm::csmString readPath = m_packet->_modelHome + m_packet->_Setting->GetUserDataFile();
		FileBuffer = QLive2dAdapter::LoadFileAsBytes(QLive2dAdapter::CsmStringToQString(readPath));
		LoadUserData(QLive2dAdapter::toCsmByte(FileBuffer), QLive2dAdapter::toCsmSize(FileBuffer));
	}

	//EyeBlink
	if (m_packet->_Setting->GetEyeBlinkParameterCount() > 0)
	{
		_eyeBlink = Csm::CubismEyeBlink::Create(m_packet->_Setting);
	}

	// EyeBlinkIds
	{
		Csm::csmInt32 eyeBlinkIdCount = m_packet->_Setting->GetEyeBlinkParameterCount();
		for (Csm::csmInt32 i = 0; i < eyeBlinkIdCount; ++i)
		{
			m_packet->_eyeBlinkIds.PushBack(m_packet->_Setting->GetEyeBlinkParameterId(i));
		}
	}

	// LipSyncIds
	{
		Csm::csmInt32 lipSyncIdCount = m_packet->_Setting->GetLipSyncParameterCount();
		for (Csm::csmInt32 i = 0; i < lipSyncIdCount; ++i)
		{
			m_packet->_lipSyncsIds.PushBack(m_packet->_Setting->GetLipSyncParameterId(i));
		}
	}

	//Breath
	{
		_breath = Csm::CubismBreath::Create();

		Csm::csmVector<Csm::CubismBreath::BreathParameterData> breathParameters;
		breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(m_packet->_idParamHeadAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
		breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(m_packet->_idParamHeadAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
		breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(m_packet->_idParamHeadAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
		breathParameters.PushBack(Csm::CubismBreath::BreathParameterData(m_packet->_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
		breathParameters.PushBack(Csm::CubismBreath::BreathParameterData
		(
			Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f)
		);

		_breath->SetParameters(breathParameters);
	}


	//Layout
	Csm::csmMap<Csm::csmString, Csm::csmFloat32> layout;
	m_packet->_Setting->GetLayoutMap(layout);
	_modelMatrix->SetupFromLayout(layout);
	_model->SaveParameters();

	_motionManager->StopAllMotions();
	_initialized = true;
	_updating = false;
	emit loadModelSucceeded();      //加载成功
	m_packet->_printTimer->start(1000 / 60.0);
	return true;
}

bool QLive2dSprite::openVoice(const QString& Path)
{
	QVoice Voice;
	if (Voice.LoadSound(Path))
	{
		m_packet->player->setMedia(QUrl::fromLocalFile(Voice.Path()));
		m_packet->player->setVolume(100); // 设置音量
		m_packet->player->play(); // 开始播放
		return true;
	}
	return false;
}

bool QLive2dSprite::setVoice(const QString& Name)
{
	if (_motionManager->IsFinished() == false) return false;

	//Name : "%s_%d"
	Csm::csmString name = QLive2dAdapter::QStringToCsmString(Name);
	QVoice* voice = m_packet->_voices[name.GetRawString()];
	if (voice == nullptr) return false;

	emit voicesChanged(Name);   //切换动作
	int spritePriority = int(m_packet->_spritePriority);
	if (m_packet->_spritePriority == Csm::CubismPriority::PriorityStrong)
	{
		_motionManager->SetReservePriority(spritePriority);
	}
	else if (!_motionManager->ReserveMotion(spritePriority))
	{
		return false;
	}
	// 播放音乐
	return openVoice(voice->Path());
}

bool QLive2dSprite::setMotion(const QString& Name)
{
	if (_motionManager->IsFinished() == false) return false;

	//Name : "%s_%d"
	Csm::csmString name = QLive2dAdapter::QStringToCsmString(Name);
	Csm::CubismMotion* motion = static_cast<Csm::CubismMotion*>(m_packet->_motions[name.GetRawString()]);
	QVoice* voice = m_packet->_voices[name.GetRawString()];
	if (motion == nullptr) return false;

	emit motionsChanged(Name);   //切换动作
	int spritePriority = int(m_packet->_spritePriority);
	if (m_packet->_spritePriority == Csm::CubismPriority::PriorityStrong)
	{
		_motionManager->SetReservePriority(spritePriority);
	}
	else if (!_motionManager->ReserveMotion(spritePriority))
	{
		return false;
	}

	if (voice) openVoice(voice->Path());// 播放音乐
	// MotionHandle是一个动作句柄 //InvalidMotionQueueEntryHandleValue是一个无动作句柄
	auto MotionHandle = _motionManager->StartMotionPriority(motion, false, spritePriority);
	return (Csm::InvalidMotionQueueEntryHandleValue != MotionHandle); 
}

bool QLive2dSprite::setExpression(const QString& Name)
{
	if (_motionManager->IsFinished() == false) return false;

	Csm::csmString name = QLive2dAdapter::QStringToCsmString(Name);

	Csm::ACubismMotion* motion = m_packet->_expressions[name];
	if (motion == nullptr) return false;

	emit expressionsChanged(Name); //切换表情
	_expressionManager->StartMotion(motion, false);
	return true;
}


void QLive2dSprite::initializeGL()
{
	//OpenGL 初始化
	this->makeCurrent();
	auto glFunction = this->context()->functions();
	glFunction->initializeOpenGLFunctions();
	int error = glewInit();
	if (error == GLEW_OK)
	{
		glFunction->glEnable(GL_DEPTH_TEST);
		glFunction->glActiveTexture(GL_TEXTURE0);
		glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glFunction->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFunction->glEnable(GL_BLEND);
		glFunction->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glFunction->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		//this->resizeGL(1080, 720);
		m_packet->_printTimer = new QTimer(this);
		connect(m_packet->_printTimer, &QTimer::timeout, [this]() { this->update(); });
	}

	auto now = std::chrono::system_clock::now();
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	m_packet->_currFrame = milliseconds / 1000.0;
	m_packet->_deltaTime = m_packet->_currFrame - m_packet->_lastFrame;
	m_packet->_lastFrame = m_packet->_currFrame;

	m_packet->_idParamHeadAngleX = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamAngleX);
	m_packet->_idParamHeadAngleY = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamAngleY);
	m_packet->_idParamHeadAngleZ = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamAngleZ);
	m_packet->_idParamBodyAngleX = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamBodyAngleX);
	m_packet->_idParamEyeBallX = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamEyeBallX);
	m_packet->_idParamEyeBallY = Csm::CubismFramework::GetIdManager()->GetId(Csm::DefaultParameterId::ParamEyeBallY);
}


void QLive2dSprite::UpdateTime()
{
	double deltaTimeSeconds = m_packet->_deltaTime;
	m_packet->_renderTimeSec += deltaTimeSeconds;

	_dragManager->Update(deltaTimeSeconds);
	_dragX = _dragManager->GetX();
	_dragY = _dragManager->GetY();

	// モーションによるパラメータ更新の有無
	Csm::csmBool motionUpdated = false;

	//-----------------------------------------------------------------
	_model->LoadParameters(); // 前回セーブされた状態をロード
	if (_motionManager->IsFinished() == false)
	{
		motionUpdated = _motionManager->UpdateMotion(_model, deltaTimeSeconds);
	}
	_model->SaveParameters(); // 状態を保存
	//-----------------------------------------------------------------

	// 不透明度
	_opacity = _model->GetModelOpacity();

	// まばたき
	if (!motionUpdated)
	{
		if (_eyeBlink != NULL)
		{
			// メインモーションの更新がないとき
			_eyeBlink->UpdateParameters(_model, deltaTimeSeconds); // 目パチ
		}
	}

	if (_expressionManager != NULL)
	{
		_expressionManager->UpdateMotion(_model, deltaTimeSeconds); // 表情でパラメータ更新（相対変化）
	}

	//ドラッグによる変化
	_model->AddParameterValue(m_packet->_idParamHeadAngleX, _dragX * 30); // -30から30の値を加える
	_model->AddParameterValue(m_packet->_idParamHeadAngleY, _dragY * 30);
	_model->AddParameterValue(m_packet->_idParamHeadAngleZ, _dragX * _dragY * -30);
	_model->AddParameterValue(m_packet->_idParamBodyAngleX, _dragX * 10); // -10から10の値を加える
	_model->AddParameterValue(m_packet->_idParamEyeBallX, _dragX); // -1から1の値を加える
	_model->AddParameterValue(m_packet->_idParamEyeBallY, _dragY);

	// 呼吸など
	if (_breath != NULL)
	{
		_breath->UpdateParameters(_model, deltaTimeSeconds);
	}

	// 物理演算の設定
	if (_physics != NULL)
	{
		_physics->Evaluate(_model, deltaTimeSeconds);
	}

	// リップシンクの設定
	if (_lipSync)
	{
		// 状態更新/RMS値取得0〜1
		//m_packet->voiceValue = 0.5f;
		//m_packet->_voiceHandler->Update(deltaTimeSeconds);
		//this->m_packet->voiceValue = m_packet->_voiceHandler->GetRms();
		for (Csm::csmUint32 i = 0; i < m_packet->_lipSyncsIds.GetSize(); ++i)
		{
			_model->AddParameterValue(m_packet->_lipSyncsIds[i], this->m_packet->voiceValue, 0.8f);
		}
	}

	// ポーズの設定
	if (_pose != NULL)
	{
		_pose->UpdateParameters(_model, deltaTimeSeconds);
	}

	_model->Update();

}

void QLive2dSprite::paintGL()
{
	if (this->GetModel())
	{
		this->makeCurrent();
		auto glFunction = this->context()->functions();
		glFunction->glClearColor(m_packet->_spriteColor[0], m_packet->_spriteColor[1], m_packet->_spriteColor[2], m_packet->_spriteColor[3]);
		glFunction->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//时间
		auto now = std::chrono::system_clock::now();
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		m_packet->_currFrame = milliseconds / 1000.0;
		m_packet->_deltaTime = m_packet->_currFrame - m_packet->_lastFrame;
		m_packet->_lastFrame = m_packet->_currFrame;

		//位置
		Csm::CubismMatrix44 projection;
		if (this->GetModel()->GetCanvasWidth() > 1.0f && width() < height())
		{
			this->GetModelMatrix()->SetWidth(2.0f);
			projection.Scale(1.0f, static_cast<float>(width()) / static_cast<float>(height()));
		}
		else
		{
			projection.Scale(static_cast<float>(height()) / static_cast<float>(width()), 1.0f);
		}
		projection.MultiplyByMatrix(GetModelMatrix());

		//绘制
		this->UpdateTime();
		this->GetRenderer<Csm::Rendering::CubismRenderer_OpenGLES2>()->SetMvpMatrix(&projection);
		this->GetRenderer<Csm::Rendering::CubismRenderer_OpenGLES2>()->DrawModel();
	}
}


void QLive2dSprite::resizeGL(int width, int height)
{
	this->makeCurrent();
	QOpenGLWidget::resizeGL(width, height);
	this->context()->functions()->glViewport(0, 0, width, height);

	float ratio = static_cast<float>(width) / static_cast<float>(height);
	float left = -ratio;
	float right = ratio;
	float bottom = +1;
	float top = -1;

	m_packet->_spriteMatrix.LoadIdentity();
	if (width > height)
	{
		float screenW = fabsf(right - left);
		m_packet->_spriteMatrix.ScaleRelative(screenW / width, -screenW / width);
	}
	else
	{
		float screenH = fabsf(top - bottom);
		m_packet->_spriteMatrix.ScaleRelative(screenH / height, -screenH / height);
	}
	m_packet->_spriteMatrix.TranslateRelative(-width * 0.5f, -height * 0.5f);
}


void QLive2dSprite::ReleaseVoices()
{
	for (auto iter = m_packet->_voices.Begin();
		iter != m_packet->_voices.End(); ++iter)
	{
		delete iter->Second;
	}

	m_packet->_voices.Clear();
}

void QLive2dSprite::ReleaseMotions()
{
	for (auto iter = m_packet->_motions.Begin();
		iter != m_packet->_motions.End(); ++iter)
	{
		Csm::ACubismMotion::Delete(iter->Second);
	}

	m_packet->_motions.Clear();
}

void QLive2dSprite::ReleaseExpressions()
{
	for (auto iter = m_packet->_expressions.Begin();
		iter != m_packet->_expressions.End(); ++iter)
	{
		Csm::ACubismMotion::Delete(iter->Second);
	}

	m_packet->_expressions.Clear();
}

QString QLive2dSprite::HitTest(float x, float y, float opa)
{
	if (_opacity >= opa)
	{
		const Csm::csmInt32 count = m_packet->_Setting->GetHitAreasCount();
		for (Csm::csmInt32 i = 0; i < count; i++)
		{
			Csm::csmString AreaName = m_packet->_Setting->GetHitAreaName(i);
			if (IsHit(m_packet->_Setting->GetHitAreaId(i), x, y))
			{
				Csm::csmString AreaName = m_packet->_Setting->GetHitAreaName(i);
				return QLive2dAdapter::CsmStringToQString(AreaName);
			}
		}
	}
	return "";
}



void QLive2dSprite::wheelEvent(QWheelEvent* Event)
{
	QOpenGLWidget::wheelEvent(Event);
}

void QLive2dSprite::paintEvent(QPaintEvent* Event)
{
	QOpenGLWidget::paintEvent(Event);
}

void QLive2dSprite::resizeEvent(QResizeEvent* Event)
{
	QOpenGLWidget::resizeEvent(Event);
}

void QLive2dSprite::mousePressEvent(QMouseEvent* Event)
{
	QOpenGLWidget::mousePressEvent(Event);
	m_packet->_touchManager.TouchesBegan(Event->x(), Event->y());
}

void QLive2dSprite::mouseReleaseEvent(QMouseEvent* Event)
{
	QOpenGLWidget::mouseReleaseEvent(Event);
	float spriteX = m_packet->_spriteMatrix.TransformX(m_packet->_touchManager.GetX());
	float spriteY = m_packet->_spriteMatrix.TransformY(m_packet->_touchManager.GetY());
	QString HitName = this->HitTest(spriteX, spriteY);

	if (HitName == "Head")
	{
		//TODO SOME THING
	}
	else if (HitName == "Body")
	{
		//TODO SOME THING
	}
	else if (HitName == "EveryThing")
	{
		//TODO SOME THING
	}
}

void QLive2dSprite::mouseMoveEvent(QMouseEvent* Event)
{
	QOpenGLWidget::mouseMoveEvent(Event);
	m_packet->_touchManager.TouchesMoved(Event->x(), Event->y());
	float spriteX = m_packet->_spriteMatrix.TransformX(m_packet->_touchManager.GetX());
	float spriteY = m_packet->_spriteMatrix.TransformY(m_packet->_touchManager.GetY());
	this->SetDragging(spriteX, spriteY);
}
