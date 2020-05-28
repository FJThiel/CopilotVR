#pragma once

#include "third_party/glm/glm.hpp"
#include <openvr.h>

static class VRUtils {
	
public:

	// Converts a matrix from the OVR format into the glm format. Also changes from row major to column major.
	static glm::mat4 OVRMatrixToGLM(vr::HmdMatrix44_t matrixIn) {
		glm::mat4 matrixOut = glm::mat4();

		for (int i = 0; i < 4; i++) {
			for (int e = 0; e < 4; e++) {
				matrixOut[e][i] = matrixIn.m[i][e];
			}
		}

		return matrixOut;
	}

	static glm::mat3 OVRMatrixToGLM(vr::HmdMatrix33_t matrixIn) {
		glm::mat3 matrixOut = glm::mat3();

		for (int i = 0; i < 3; i++) {
			for (int e = 0; e < 3; e++) {
				matrixOut[e][i] = matrixIn.m[i][e];
			}
		}

		return matrixOut;
	}

	static glm::mat4x3 OVRMatrixToGLM(vr::HmdMatrix34_t matrixIn) {
		glm::mat4x3 matrixOut = glm::mat4x3();

		for (int i = 0; i < 3; i++) {
			for (int e = 0; e < 4; e++) {
				matrixOut[e][i] = matrixIn.m[i][e];
			}
		}

		return matrixOut;
	}

	static vr::HmdMatrix44_t GLMMAtrixToOVR(glm::mat4 matrixIn) {
		vr::HmdMatrix44_t matrixOut = vr::HmdMatrix44_t();

		for (int i = 0; i < 4; i++) {
			for (int e = 0; e < 4; e++) {
				matrixOut.m[e][i] = matrixIn[i][e];
			}
		}

		return matrixOut;
	}

	static vr::HmdMatrix33_t GLMMAtrixToOVR(glm::mat3 matrixIn) {
		vr::HmdMatrix33_t matrixOut = vr::HmdMatrix33_t();

		for (int i = 0; i < 3; i++) {
			for (int e = 0; e < 3; e++) {
				matrixOut.m[e][i] = matrixIn[i][e];
			}
		}

		return matrixOut;
	}

	static vr::HmdMatrix34_t GLMMAtrixToOVR(glm::mat4x3 matrixIn) {
		vr::HmdMatrix34_t matrixOut = vr::HmdMatrix34_t();

		for (int i = 0; i < 4; i++) {
			for (int e = 0; e < 3; e++) {
				matrixOut.m[e][i] = matrixIn[i][e];
			}
		}

		return matrixOut;
	}

};