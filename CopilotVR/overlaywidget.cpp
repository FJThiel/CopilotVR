#include "overlaywidget.h"
#include "ui_overlaywidget.h"
#include <iostream>

OverlayWidget::OverlayWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::OverlayWidget)
{
	m_ui->setupUi(this);

	// set preset values in the ui
	m_ui->movLimitSpinBox_XZ->setValue(static_cast<double>(m_maxOffset.x));
	m_ui->movLimitSpinBox_Y->setValue(static_cast<double>(m_maxOffset.y));
	m_ui->rtc_checkBox->setChecked(m_returnToCentre);
	


	// QT Timer for the input loop
	m_timer = std::make_shared<QTimer>(this);

	connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(update()));

	m_timer->setInterval(m_timerInputInterval);
	m_timer->start();




	// SteamVR actions initialization

	// Set Path to actions manifest
	std::string curPath = std::experimental::filesystem::v1::current_path().string();
	std::string manifestPath = curPath.append("\\SteamVROverlayWidget\\actions.json");
	
	vr::EVRInputError error = vr::VRInput()->SetActionManifestPath(manifestPath.c_str());

	// get action handles (digital)
	error = vr::VRInput()->GetActionHandle("/actions/main/in/digitalMoveLeft", &m_handleDigitalMoveLeft);
	vr::VRInput()->GetActionHandle("/actions/main/in/digitalMoveRight", &m_handleDigitalMoveRight);
	vr::VRInput()->GetActionHandle("/actions/main/in/digitalMoveForward", &m_handleDigitalMoveForward);
	vr::VRInput()->GetActionHandle("/actions/main/in/digitalMoveBackward", &m_handleDigitalMoveBackward);
	vr::VRInput()->GetActionHandle("/actions/main/in/digitalDuck", &m_handleDigitalDuck);
	vr::VRInput()->GetActionHandle("/actions/main/in/digitalReset", &m_handleDigitalReset);
	
	// get action handles (analogue)
	vr::VRInput()->GetActionHandle("/actions/main/in/analogueMovement", &m_handleAnalogueMovement);
	vr::VRInput()->GetActionHandle("/actions/main/in/analogueDuck", &m_handleAnalogueDuck);
	vr::VRInput()->GetActionHandle("/actions/main/in/analogueRotation", &m_handleAnalogueRotation);
	

	// get action set handle
	vr::VRInput()->GetActionSetHandle("/actions/main", &m_handleSetMain);


	// get initial setup for later cleanup
	vr::VRChaperoneSetup()->HideWorkingSetPreview();
	vr::VRChaperoneSetup()->RevertWorkingCopy(); // Working copy is updated with live values
	vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&m_initialMatrix);


	// Init inputemulator	
	std::cout << "Looking for VR Input Emulator..." << std::flush;
	while (true) {
		try {
			m_inputEmulator.connect();
			break;
		}
		catch (vrinputemulator::vrinputemulator_connectionerror e) {
			std::cout << "\nFailed to connect to open vr input emulator, ensure you've installed it. If you have, try running this fix: https://drive.google.com/open?id=1Gn3IOm6GbkINplbEenu0zTr3DkB1E8Hc \n" << std::flush;
			std::this_thread::sleep_for(std::chrono::seconds(4));
			continue;
		}
	}
	std::cout << "Success!\n";




	// get initial offsets
	std::cout << "Retreiving initial offsets. Please put on HMD..." << std::flush;

	bool success = false;
	vr::HmdMatrix34_t currentPosRotRaw;

	while (!success) {
		vr::VRChaperoneSetup()->RevertWorkingCopy(); // Working copy is updated with live values		
		success = vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&currentPosRotRaw);

		// if we are not successful, wait a second then try again
		if (!success)
			std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "Success!\n";
	

	// convert raw into glm format and then to a 4x4 rotation-only matrix
	glm::mat4x3 currentPosRot = VRUtils::OVRMatrixToGLM(currentPosRotRaw);
	glm::mat4 currentRot = glm::mat4(1.f);
	currentRot[0] = glm::vec4(currentPosRot[0], 0.f);
	currentRot[1] = glm::vec4(currentPosRot[1], 0.f);
	currentRot[2] = glm::vec4(currentPosRot[2], 0.f);

	m_initialTranslation = currentPosRot[3];
	m_initialRotationMatrix = currentRot;
	

	for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex)) {
			m_deviceBaseOffsets[deviceIndex] = glm::vec3(0);
			continue;
		}
		vrinputemulator::DeviceOffsets data;
		try {
			m_inputEmulator.getDeviceOffsets(deviceIndex, data);
			glm::vec3 offset;
			offset.x = (float)data.worldFromDriverTranslationOffset.v[0];
			offset.y = (float)data.worldFromDriverTranslationOffset.v[1];
			offset.z = (float)data.worldFromDriverTranslationOffset.v[2];
			m_deviceBaseOffsets[deviceIndex] = offset;
			m_deviceBaseRotations[deviceIndex] = data.worldFromDriverRotationOffset;
			
			//auto test1 = data.worldFromDriverRotationOffset;
			//auto test2 = data.driverFromHeadRotationOffset;
			//auto test3 = data.deviceRotationOffset;

			auto test1 = data.worldFromDriverTranslationOffset;
			auto test2 = data.driverFromHeadTranslationOffset;
			auto test3 = data.deviceTranslationOffset;

			int e = 5;
			
		}
		catch (vrinputemulator::vrinputemulator_notfound e) {
			glm::vec3 offset;
			offset.x = 0;
			offset.y = 0;
			offset.z = 0;
			m_deviceBaseOffsets[deviceIndex] = offset;
			m_deviceBaseRotations[deviceIndex] = vr::HmdQuaternion_t();
		}
	}
	
	int i = 5;
}

OverlayWidget::~OverlayWidget()
{
	delete m_ui;

	ResetChaperoneOffset();

	m_inputEmulator.disconnect();

	//// Reset steamVR pose
	//vr::VRChaperoneSetup()->HideWorkingSetPreview();
	//vr::VRChaperoneSetup()->RevertWorkingCopy(); // Working copy is updated with live values
	//vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&m_initialMatrix);
	//vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
}

void OverlayWidget::on_pushButton_clicked()
{
	QApplication::quit();
}

void OverlayWidget::on_pushButton_2_clicked()
{
	m_ui->pushButton_2->setText("Click!");
}

void OverlayWidget::on_pushButton_up_clicked()
{
	glm::vec3 offset = glm::vec3(0.f, 0.25f, 0.f );
	SetChaperoneOffsetExp(offset, 0.f, false);
}

void OverlayWidget::on_pushButton_down_clicked()
{
	glm::vec3 offset = glm::vec3(0.f, -0.25f, 0.f );
	SetChaperoneOffsetExp(offset, 0.f, false);
}

void OverlayWidget::on_pushButton_left_clicked()
{
	glm::vec3 offset = glm::vec3( -0.25f, 0.f, 0.f );
	SetChaperoneOffsetExp(offset, 0.f, false);
}

void OverlayWidget::on_pushButton_right_clicked()
{
	glm::vec3 offset = glm::vec3( 0.25f, 0.f, 0.f );
	SetChaperoneOffsetExp(offset, 0.f, false);
}

void OverlayWidget::on_pushButton_refreshDevices_clicked()
{
	// we have to query all indices and see where we get feedback
	// see listDevices here: https://github.com/matzman666/OpenVR-InputEmulator/blob/de7e1ec9ab1f0aa5766d58ac65db061ba6615d2d/client_commandline/src/client_commandline.cpp

	m_devices.clear();

	// for each possible device index
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {

		// get the device class of the index to check if anybody is at home
		auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(static_cast<vr::TrackedDeviceIndex_t>(i));

		// check if something is connected there
		if (deviceClass != vr::TrackedDeviceClass_Invalid) {

			//someone is, so get data and create output
			vr::ETrackedPropertyError pError;

			char serial[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_SerialNumber_String, serial, 1028, &pError);

			char manufacturer[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ManufacturerName_String, manufacturer, 1028, &pError);

			char model[1028] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ModelNumber_String, model, 1028, &pError);

			std::string deviceClassStr;

			if (deviceClass == vr::TrackedDeviceClass_HMD) {
				deviceClassStr = "HMD";
			}
			else if (deviceClass == vr::TrackedDeviceClass_Controller) {
				deviceClassStr = "Controller";
			}
			else if (deviceClass == vr::TrackedDeviceClass_GenericTracker) {
				deviceClassStr = "Generic Tracker";
			}
			else if (deviceClass == vr::TrackedDeviceClass_TrackingReference) {
				deviceClassStr = "Tracking Reference";
			}
			else {
				deviceClassStr = "Unknown";
			}

			std::shared_ptr<TrackedDevice> device = std::make_shared<TrackedDevice>();
			device->setDeviceIndex(i);
			device->setSerial(serial);
			device->setManufacturer(manufacturer);
			device->setModel(model);
			device->setDeviceClassStr(deviceClassStr);

			m_devices.push_back(device);
		}

	}

	// print all device model strings in the list box
	m_ui->listWidget->clear();

	for (int i = 0; i < m_devices.size(); i++) {
		QString string = QString::fromStdString(m_devices[i]->getModel());
		m_ui->listWidget->addItem(string);
	}
}

void OverlayWidget::on_listWidget_currentRowChanged(int currentRow)
{
	// check if row is safe and abort if not
	if (currentRow < 0 || currentRow >= m_devices.size())
		return;

	// get associated device
	std::shared_ptr<TrackedDevice> device = m_devices[currentRow];

	// clear label
	m_ui->label->clear();

	// Get data from device and parse it to Qstrings
	QString deviceIndex = QString::number(device->getDeviceIndex());
	QString serial = QString::fromStdString(device->getSerial());
	QString manufacturer = QString::fromStdString(device->getManufacturer());
	QString model = QString::fromStdString(device->getModel());
	QString deviceClassString = QString::fromStdString(device->getDeviceClassStr());

	// Format and put in the label
	QString content = "";
	content = content + "Index: " + deviceIndex;
	content = content + "\nSerial: " + serial;
	content = content + "\nManufacturer: " + manufacturer;
	content = content + "\nModel: " + model;
	content = content + "\nDeviceClassString: " + deviceClassString;

	m_ui->label->setText(content);

	

}

glm::vec3 OverlayWidget::getRawInputDigital() {
	
	bool leftPressed = false;
	bool rightPressed = false;
	bool forwardPressed = false;
	bool backwardPressed = false;
	bool duckPressed = false;

	{
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalMoveLeft, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		leftPressed = actionData.bState && actionData.bActive;
	}

	{
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalMoveRight, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		rightPressed = actionData.bState && actionData.bActive;
	}

	{
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalMoveForward, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		forwardPressed = actionData.bState && actionData.bActive;
	}

	{
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalMoveBackward, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		backwardPressed = actionData.bState && actionData.bActive;
	}

	{
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalDuck, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		duckPressed = actionData.bState && actionData.bActive;
	}

	// code to move the player according to the action

	// calculate contribution for every direction
	float leftContribution = leftPressed ? m_step : 0.f;
	float rightContribution = rightPressed ? -m_step : 0.f;
	float forwardContribution = forwardPressed ? m_step : 0.f;
	float backwardContribution = backwardPressed ? -m_step : 0.f;
	float duckContribution = duckPressed ? m_step : 0.f;

	glm::vec3 movement = glm::vec3(0,0,0);
	movement.x = leftContribution + rightContribution;
	movement.y = duckContribution;
	movement.z = forwardContribution + backwardContribution;

	// inverse, because we actually move the world and not the player
	movement = -movement;

	return movement;
}

glm::vec3 OverlayWidget::getRawInputAnalogue() {

	float inputXMovement = 0;
	float inputYMovement = 0;
	float inputZMovement = 0;

	{
		vr::InputAnalogActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetAnalogActionData(m_handleAnalogueMovement, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);

		if (actionData.bActive) {
			inputXMovement = actionData.x;
			inputZMovement = actionData.y;
		}
	}

	{
		vr::InputAnalogActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetAnalogActionData(m_handleAnalogueDuck, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);

		if (actionData.bActive) {
			inputYMovement = actionData.x;
		}
	}

	// deadzone (everything below m_deadzone gets set to zero)
	if (inputXMovement > -m_deadzone && inputXMovement < m_deadzone)
		inputXMovement = 0.f;
	if (inputYMovement > -m_deadzone && inputYMovement < m_deadzone)
		inputYMovement = 0.f;
	if (inputZMovement > -m_deadzone && inputZMovement < m_deadzone)
		inputZMovement = 0.f;

	glm::vec3 input = glm::vec3( 0, 0, 0 );

	input.x = -inputXMovement * m_step; // mirrored input
	input.y = inputYMovement * m_step;
	input.z = inputZMovement * m_step;

	// mirror all, as we actually want to move the world not the player
	input = -input;

	return input;
}

float OverlayWidget::getRotationAnalogue() {

	float inputXMovement = 0.f;

	vr::InputAnalogActionData_t actionData;
	vr::EVRInputError error = vr::VRInput()->GetAnalogActionData(m_handleAnalogueRotation, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);

	if (actionData.bActive) {
		inputXMovement = actionData.x;		
	}

	// deadzone (everything below m_deadzone gets set to zero)
	if (inputXMovement > -m_deadzone && inputXMovement < m_deadzone)
		inputXMovement = 0.f;

	return -inputXMovement * (m_step); // mirrored as we actually want to move the world not the player
}

glm::vec3 OverlayWidget::processInput(glm::vec3 rawInput) {

	// rotate with the current rotation so it is relative to the current direction
	glm::vec4 trans4 = glm::vec4(rawInput, 0.f);
	glm::mat4 rot = glm::rotate(glm::mat4(1.f), m_currentRotation, glm::vec3(0.f, 1.f, 0.f));
	//trans4 = rot * trans4;
	//trans4 = m_initialRotationMatrix * trans4;
	trans4 = m_initialRotationMatrix * rot * trans4; // not effective yet
	glm::vec3 translation = glm::vec3(trans4);


	//// return to centre if no other input is given
	//if(glm::length(translation) == 0.f){

	//    if(glm::length(m_currentOffset) > m_step){
	//        translation = m_step * (-glm::normalize(m_currentOffset));
	//    }
	//    else if (glm::length(m_currentOffset) < m_step){
	//        translation = -m_currentOffset;
	//    }
	//    // Do nothing if we are already at zero
	//}

	// alternative approach to return-to-centre

	if (m_returnToCentre == true)
	{
		// create mask
		glm::vec3 mask = glm::vec3(0.f);
		mask.x = translation.x == 0.f ? 1.f : 0.f;
		mask.y = translation.y == 0.f ? 1.f : 0.f;
		mask.z = translation.z == 0.f ? 1.f : 0.f;

		// "flatten" current offset vector so we get only the dimensions without change
		glm::vec3 flattenedCur = m_currentOffset * mask;
		glm::vec3 flattendedTrans = glm::vec3(0.f);

		if (glm::length(flattenedCur) > m_step) {
			flattendedTrans = m_step * (-glm::normalize(flattenedCur));
		}
		else if (glm::length(flattenedCur) < m_step) {
			flattendedTrans = -flattenedCur;
		}
		// Do nothing if we are already at zero

		// apply it back to the original translation
		translation = translation + (mask * flattendedTrans);
	}
	else
	{
		// for now, still have rtc in the y axis, as we do not have a separate button for up at the moment. This will be changed at a later time
		if (translation.y == 0.f) {
			if (m_currentOffset.y >= m_step) {
				translation.y = -m_step;
			}
			else if (m_currentOffset.y < m_step && m_currentOffset.y > 0.f) {
				translation.y = -m_currentOffset.y;
			}
			else if (m_currentOffset.y > -m_step && m_currentOffset.y < 0.f) {
				translation.y = -m_currentOffset.y;
			}
			else if (m_currentOffset.y <= -m_step) {
				translation.y = m_step;
			}
			// Do nothing if we are already at zero	
		}
	}


	//// return to centre if no other input is given
	//if (translation.x == 0.f) {

	//	if (m_currentOffset.x >= m_step) {
	//		translation.x = -m_step;
	//	}
	//	else if (m_currentOffset.x < m_step && m_currentOffset.x > 0.f) {			
	//			translation.x = -m_currentOffset.x;		
	//	}
	//	else if (m_currentOffset.x > -m_step && m_currentOffset.x < 0.f) {
	//			translation.x = -m_currentOffset.x;
	//	}
	//	else if (m_currentOffset.x <= -m_step) {
	//		translation.x = m_step;
	//	}
	//	// Do nothing if we are already at zero
	//}
	//if (translation.y == 0.f) {

	//	if (m_currentOffset.y >= m_step) {
	//		translation.y = -m_step;
	//	}
	//	else if (m_currentOffset.y < m_step && m_currentOffset.y > 0.f) {
	//		translation.y = -m_currentOffset.y;
	//	}
	//	else if (m_currentOffset.y > -m_step && m_currentOffset.y < 0.f) {
	//		translation.y = -m_currentOffset.y;
	//	}
	//	else if (m_currentOffset.y <= -m_step) {
	//		translation.y = m_step;
	//	}
	//	// Do nothing if we are already at zero
	//}
	//if (translation.z == 0.f) {

	//	if (m_currentOffset.z >= m_step) {
	//		translation.z = -m_step;
	//	}
	//	else if (m_currentOffset.z < m_step && m_currentOffset.z > 0.f) {
	//		translation.z = -m_currentOffset.z;
	//	}
	//	else if (m_currentOffset.z > -m_step && m_currentOffset.z < 0.f) {
	//		translation.z = -m_currentOffset.z;
	//	}
	//	else if (m_currentOffset.z <= -m_step) {
	//		translation.z = m_step;
	//	}
	//	// Do nothing if we are already at zero
	//}

	// clamp
	glm::vec3 newOffest = m_currentOffset + translation;

	glm::vec3 currentOffsetXZ = glm::vec3(m_currentOffset.x, 0.f, m_currentOffset.z);
	glm::vec3 newOffsetXZ = glm::vec3(newOffest.x, 0.f, newOffest.z);
	if(glm::length(newOffsetXZ) > m_maxOffset.x){
		newOffsetXZ = glm::normalize(newOffsetXZ) * m_maxOffset.x;
		glm::vec3 newTranslationXZ = newOffsetXZ - currentOffsetXZ;
		translation.x = newTranslationXZ.x;
		translation.z = newTranslationXZ.z;
	}

	if(newOffest.y > m_maxOffset.y){
		translation.y = m_maxOffset.y - m_currentOffset.y;
	}
	else if (newOffest.y < -m_maxOffset.y){
		translation.y = -m_maxOffset.y - m_currentOffset.y;
	}

	return translation;
}

void OverlayWidget::printMatrix(glm::mat4x3 m)
{
	if (!m_log)
		return;

	std::cout << std::endl;
	std::cout << m[0][0] << " | " << m[1][0] << " | " << m[2][0] << " | " << m[3][0] << std::endl;
	std::cout << m[0][1] << " | " << m[1][1] << " | " << m[2][1] << " | " << m[3][1] << std::endl;
	std::cout << m[0][2] << " | " << m[1][2] << " | " << m[2][2] << " | " << m[3][2] << std::endl;
}

void OverlayWidget::printMatrix(glm::mat4x4 m)
{
	if (!m_log)
		return;

	std::cout << std::endl;
	std::cout << m[0][0] << " | " << m[1][0] << " | " << m[2][0] << " | " << m[3][0] << std::endl;
	std::cout << m[0][1] << " | " << m[1][1] << " | " << m[2][1] << " | " << m[3][1] << std::endl;
	std::cout << m[0][2] << " | " << m[1][2] << " | " << m[2][2] << " | " << m[3][2] << std::endl;
	std::cout << m[0][3] << " | " << m[1][3] << " | " << m[2][3] << " | " << m[3][3] << std::endl;
}

void OverlayWidget::printDevices() {
	std::cout << "Started discovering device classes:" << std::endl;
	for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex))
			continue;

		vr::ETrackedPropertyError pError;

		vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);
		char controllerType[1028] = { '\0' };
		vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, vr::ETrackedDeviceProperty::Prop_ControllerType_String, controllerType, 1028, &pError);
		char manufacturer[1028] = { '\0' };
		vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, manufacturer, 1028, &pError);
		char model[1028] = { '\0' };
		vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, model, 1028, &pError);

		std::string deviceClassString = "Unidentified";

		switch (deviceClass) {
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
			deviceClassString = "Controller";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
			deviceClassString = "DisplayRedirect";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
			deviceClassString = "GenericTracker";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
			deviceClassString = "HMD";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid:
			deviceClassString = "Invalid";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Max:
			deviceClassString = "Max";
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
			deviceClassString = "TrackingReference";
			break;
		}

		std::cout << std::endl;
		std::cout << "Index: " << deviceIndex << std::endl;
		std::cout << "Class: " << deviceClassString << std::endl;
		std::cout << "Controller Type: " << controllerType << std::endl;
		std::cout << "Manufacturer: " << manufacturer << std::endl;
		std::cout << "Model Number: " << model << std::endl;
	}

	std::cout << "Completed discovery" << std::endl << std::endl;
}

// Input processing, do it here
void OverlayWidget::update() {

	// Update the action set state
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_handleSetMain;
	vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

	// check reset button separately	
		vr::InputDigitalActionData_t actionData;
		vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handleDigitalReset, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
		if (actionData.bState && actionData.bActive) {
			ResetChaperoneOffset();
		}
	

	// Get input from actions	
	glm::vec3 digitalInputMovement = getRawInputDigital();
	glm::vec3 analogueInputMovement = getRawInputAnalogue();
	float rotation = getRotationAnalogue();

	glm::vec3 rawMovement = digitalInputMovement + analogueInputMovement;
	
	glm::vec3 processedMovement = processInput(rawMovement);

	if (m_log)
	{
		std::cout << "Raw length: " << glm::length(rawMovement) << std::endl;
		std::cout << "Processed length: " << glm::length(processedMovement) << std::endl << std::endl;
	}

	// apply (if there is something to apply)
	if (processedMovement.x != 0.f || processedMovement.y != 0.f || processedMovement.z != 0.f || rotation != 0.f) {

		//SetChaperoneOffset(processedMovement, rotation, false);

		m_currentOffset = m_currentOffset + processedMovement;
		m_currentRotation = m_currentRotation + rotation;

		SetChaperoneOffsetExp(m_currentOffset, m_currentRotation, false);
	}


}

void OverlayWidget::ResetChaperoneOffset() {

	// Reset steamVR pose
	vr::VRChaperoneSetup()->HideWorkingSetPreview();
	vr::VRChaperoneSetup()->RevertWorkingCopy(); // Working copy is updated with live values
	vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&m_initialMatrix);
	//vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
	vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Temp);
	
	m_currentOffset = glm::vec3(0.f);
	m_currentRotation = 0.f;
}

std::string OverlayWidget::getStringProperty(vr::TrackedDeviceIndex_t deviceIndex, vr::ETrackedDeviceProperty deviceProperty) {
	vr::ETrackedPropertyError pError;
	char propertyChar[1028] = { '\0' };
	vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, deviceProperty, propertyChar, 1028, &pError);

	return std::string(propertyChar);
}

void OverlayWidget::SetChaperoneOffsetExp(glm::vec3 translation, float rotation, bool moveBounds) {
	
	glm::vec3 hmdPos = glm::vec3(0.f);
	
	////////////////////////////
	// get all devices positions
	////////////////////////////

	// timing stuff
	float fSecondsSinceLastVsync;
	vr::VRSystem()->GetTimeSinceLastVsync(&fSecondsSinceLastVsync, NULL);
	float fDisplayFrequency = vr::VRSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
	float fFrameDuration = 1.f / fDisplayFrequency;
	float fVsyncToPhotons = vr::VRSystem()->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SecondsFromVsyncToPhotons_Float);
	float fPredictedSecondsFromNow = fFrameDuration - fSecondsSinceLastVsync + fVsyncToPhotons;

	// Querry all poses
	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
	vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, fPredictedSecondsFromNow, devicePoses, vr::k_unMaxTrackedDeviceCount);

	// storage for all device positions
	glm::vec3 devicePos[vr::k_unMaxTrackedDeviceCount];

	for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex))
			continue;

		vr::TrackedDevicePose_t* pose = devicePoses + deviceIndex;
		vr::HmdMatrix34_t* poseMat = &(pose->mDeviceToAbsoluteTracking);
		if (pose->bPoseIsValid && pose->bDeviceIsConnected) {			
			devicePos[deviceIndex] = glm::vec3(poseMat->m[0][3], poseMat->m[1][3], poseMat->m[2][3]);
		}

		// We want to store the hmd pos seperately
		vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);
		if (deviceClass == vr::ETrackedDeviceClass::TrackedDeviceClass_HMD) {			
			hmdPos = devicePos[deviceIndex];
		}

	}




	for (uint32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
		if (!vr::VRSystem()->IsTrackedDeviceConnected(deviceIndex))
			continue;

		std::string controllerType = getStringProperty(deviceIndex, vr::ETrackedDeviceProperty::Prop_ControllerType_String);
		
		// exclude Liv cam
		if (controllerType == "liv_virtualcamera")
			continue;


		// temporary: only the controller are moved now
		// used for tracking down issues with controller rotation
		//vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);
		//if (deviceClass != vr::ETrackedDeviceClass::TrackedDeviceClass_Controller) {
		//	continue;
		//}
		

		// calculate rotatiom matrix
		// glm::vec3 actualCentre = -m_initialTranslation; more like a note reminding me where the actual centre lies
		//glm::vec3 perceivedCentre = glm::vec3(0.f);
		//glm::vec3 fromHmdToDevice = devicePos[deviceIndex] - hmdPos; // not used yet
		glm::mat4 fromActualToPerceived = glm::translate(glm::mat4(1.f), m_initialTranslation);
		glm::mat4 rotMat = glm::rotate(glm::mat4(1.f), rotation, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 fromPerceivedToActual = glm::translate(glm::mat4(1.f), -m_initialTranslation);		
		glm::mat4 rotation = fromActualToPerceived * rotMat * fromPerceivedToActual;
		

		//rotation = rotMat;

		m_inputEmulator.enableDeviceOffsets(deviceIndex, true, false);

		// Apply Rotation
		glm::fquat quat = glm::quat_cast(rotation);
		vr::HmdQuaternion_t copyRotation;
		copyRotation.x = quat.x;
		copyRotation.y = quat.y;
		copyRotation.z = quat.z;
		copyRotation.w = quat.w;

		m_inputEmulator.setWorldFromDriverRotationOffset(deviceIndex, copyRotation, false);

		// Apply translation
		glm::vec3 rotationalTranslation = glm::vec3(rotation[3]);
		translation = rotationalTranslation + translation;

		vr::HmdVector3d_t copyTranslation;
		copyTranslation.v[0] = translation.x;
		copyTranslation.v[1] = translation.y;
		copyTranslation.v[2] = translation.z;
		
		m_inputEmulator.setWorldFromDriverTranslationOffset(deviceIndex, copyTranslation, false);
		
	}

}

void OverlayWidget::SetChaperoneOffset(glm::vec3 translation, float rotation, bool moveBounds)
{
	
	vr::VRChaperoneSetup()->HideWorkingSetPreview();

	vr::VRChaperoneSetup()->RevertWorkingCopy(); // Working copy is updated with live values

	// get values 
	vr::HmdMatrix34_t currentPosRaw;
	bool success = vr::VRChaperoneSetup()->GetWorkingStandingZeroPoseToRawTrackingPose(&currentPosRaw);

	if (!success)
		return;

	// convert raw into glm format
	glm::mat4x3 currentPos = VRUtils::OVRMatrixToGLM(currentPosRaw);

	// create transformation matrices
	glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.f, 1.f, 0.f));


	// apply transformation matrics
	currentPos = currentPos * translationMat * rotationMat;
	
	// convert back to raw
	currentPosRaw = VRUtils::GLMMAtrixToOVR(currentPos);
	   
	vr::VRChaperoneSetup()->SetWorkingStandingZeroPoseToRawTrackingPose(&currentPosRaw);

	// do we want to move the bounds as well?
	// If not, we have to apply a negative offset on them
	if (moveBounds == false)
	{
		// first query how many vertices the bound has
		unsigned collisionBoundsCount = 0;
		vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(nullptr, &collisionBoundsCount);

		if (collisionBoundsCount > 0)
		{
			// now assign memory and get bounds themselves
			vr::HmdQuad_t* collisionBounds = new vr::HmdQuad_t[collisionBoundsCount];
			
			vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(collisionBounds, &collisionBoundsCount);

			// iterate over all of them and move them as requested
			for (unsigned b = 0; b < collisionBoundsCount; b++)
			{
				for (unsigned c = 0; c < 4; c++)
				{
					collisionBounds[b].vCorners[c].v[0] -= translation[0];

					// keep lower coordinates on the ground as OpenVR sanity
					// checks the y coordinates. If they are not 0, it get's reset
					// to default. We don't want that.
					if (collisionBounds[b].vCorners[c].v[1] != 0.f)
					{
						collisionBounds[b].vCorners[c].v[1] -= translation[1];
					}

					collisionBounds[b].vCorners[c].v[2] -= translation[2];
				}
			}

			// apply and clean up
			vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(collisionBounds, collisionBoundsCount);
			delete[] collisionBounds;
		}
	}


	// commit
	//vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
	vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Temp);

}

void OverlayWidget::SetCollisionBoundsOffset(float offset[3])
{
	// very similar to when we want to manipulate the whole playspace


   // align working copy with original
	vr::VRChaperoneSetup()->HideWorkingSetPreview();
	vr::VRChaperoneSetup()->RevertWorkingCopy();

	// first query how many vertices the bound has
	unsigned collisionBoundsCount = 0;
	vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(nullptr, &collisionBoundsCount);

	if (collisionBoundsCount > 0)
	{
		// now assign memory and get bounds themselves
		vr::HmdQuad_t* collisionBounds = new vr::HmdQuad_t[collisionBoundsCount];
		vr::VRChaperoneSetup()->GetWorkingCollisionBoundsInfo(collisionBounds, &collisionBoundsCount);

		// iterate over all of them and move them as requested
		for (unsigned b = 0; b < collisionBoundsCount; b++)
		{
			for (unsigned c = 0; c < 4; c++)
			{
				collisionBounds[b].vCorners[c].v[0] += offset[0];

				// keep lower coordinates on the ground as OpenVR sanity
				// checks the y coordinates. If they are not 0, it get's reset
				// to default. We don't want that.
				if (collisionBounds[b].vCorners[c].v[1] != 0.f)
				{
					collisionBounds[b].vCorners[c].v[1] += offset[1];
				}

				collisionBounds[b].vCorners[c].v[2] += offset[2];
			}
		}

		// apply and clean up
		vr::VRChaperoneSetup()->SetWorkingCollisionBoundsInfo(collisionBounds, collisionBoundsCount);
		delete[] collisionBounds;

		// commit
		//vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Live);
		vr::VRChaperoneSetup()->CommitWorkingCopy(vr::EChaperoneConfigFile_Temp);
	}
}

void OverlayWidget::on_movLimitSpinBox_XZ_valueChanged(double newMaxOffsetX)
{
	m_maxOffset.x = static_cast<float>(newMaxOffsetX);	
}

void OverlayWidget::on_movLimitSpinBox_Y_valueChanged(double newMaxOffsetY)
{
	m_maxOffset.y = static_cast<float>(newMaxOffsetY);
}

void OverlayWidget::on_rtc_checkBox_stateChanged(int arg1)
{
	// arg1 is actually a enum Qt::CheckState
	// https://doc.qt.io/qt-5/qt.html#CheckState-enum

	if (arg1 == 0) 
	{
		m_returnToCentre = false;
	}
	else
	{
		m_returnToCentre = true;
	}	
}
