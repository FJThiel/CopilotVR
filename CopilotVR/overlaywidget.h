#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QtWidgets/QWidget>
#include <openvr.h>
#include "trackeddevice.h"
#include <qtimer.h>
#include <QVector3D>

#include "third_party/glm/glm.hpp"
#include "third_party/glm/matrix.hpp"
#include "third_party/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/quaternion.hpp"

#include "third_party/OpenVR-InputEmulator/lib_vrinputemulator/include/vrinputemulator.h"

#include "vrutils.h"

#include <filesystem>

namespace Ui {
class OverlayWidget;
}

class OverlayWidget : public QWidget
{
	Q_OBJECT

public:
	explicit OverlayWidget(QWidget *parent = 0);
	~OverlayWidget();

private slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_up_clicked();
	void on_pushButton_down_clicked();
	void on_pushButton_left_clicked();
	void on_pushButton_right_clicked();
	void on_pushButton_refreshDevices_clicked();
	void on_listWidget_currentRowChanged(int currentRow);
	void update();

	void on_movLimitSpinBox_XZ_valueChanged(double arg1);

	void on_movLimitSpinBox_Y_valueChanged(double arg1);

	void on_rtc_checkBox_stateChanged(int arg1);

private:
	void ResetChaperoneOffset();
	void SetChaperoneOffset(glm::vec3 translation, float rotation, bool moveBounds);
	void SetChaperoneOffsetExp(glm::vec3 translation, float rotation, bool moveBounds);
	void SetCollisionBoundsOffset(float offset[3]);

	glm::vec3 getRawInputDigital();
	glm::vec3 getRawInputAnalogue();

	glm::vec3 processInput(glm::vec3 rawInput);

	float getRotationAnalogue();

	// Wrapper helper functions
	std::string getStringProperty(vr::TrackedDeviceIndex_t deviceIndex, vr::ETrackedDeviceProperty deviceProperty);

	// Debug helper function
	void printMatrix(glm::mat4x3);
	void printMatrix(glm::mat4x4);
	void printDevices();

	Ui::OverlayWidget *m_ui;
	std::vector<std::shared_ptr<TrackedDevice>> m_devices;

	std::shared_ptr<QTimer> m_timer;

	vrinputemulator::VRInputEmulator m_inputEmulator;
	glm::vec3 m_deviceBaseOffsets[vr::k_unMaxTrackedDeviceCount];
	vr::HmdQuaternion_t m_deviceBaseRotations[vr::k_unMaxTrackedDeviceCount];

	// action handles (digital)
	vr::VRActionHandle_t m_handleDigitalMoveLeft;
	vr::VRActionHandle_t m_handleDigitalMoveRight;
	vr::VRActionHandle_t m_handleDigitalMoveForward;
	vr::VRActionHandle_t m_handleDigitalMoveBackward;
	vr::VRActionHandle_t m_handleDigitalDuck;
	vr::VRActionHandle_t m_handleDigitalReset;

	// action handles (analogue)
	vr::VRActionHandle_t m_handleAnalogueMovement;
	vr::VRActionHandle_t m_handleAnalogueRotation;
	vr::VRActionHandle_t m_handleAnalogueDuck;

	vr::VRActionSetHandle_t m_handleSetMain;

	glm::vec3 m_currentOffset = glm::vec3(0.f);
	float m_currentRotation = 0.f;
	glm::vec3 m_initialTranslation = glm::vec3();
	glm::mat4 m_initialRotationMatrix = glm::mat4(1.f);

  
	vr::HmdMatrix34_t m_initialMatrix;

	glm::vec2 m_maxOffset = glm::vec2(15.0f, 1.0f);

	bool m_returnToCentre = true;

	// constant parameters
	const float m_deadzone = 0.1f;
	const float m_timerInputInterval = 0.5;
	const float m_step = 1.f / static_cast<float>(500.f / m_timerInputInterval);	// should move one meter in half a second
																				// the step should become 1 if the interval is 500 milliseconds

	const bool m_log = false;
};

#endif // OVERLAYWIDGET_H
