/// Emil Hedemalm
/// 2015-01-28
/// Old code moved for clarity..


	/*
	/// Check for xbox controller input on windows and xbox o-o
#ifdef WINDOWS
	for (int i = 0; i < 4; ++i){
		XINPUT_STATE tmpXInputState;
		DWORD result = XInputGetState(i, &tmpXInputState);

		// Skip if failed
		if (result != ERROR_SUCCESS)
			continue;
	//	std::cout<<"\nXbox Controller "<<i+1<<" responding as intended? >:3";

		static DWORD lastPacketNumber[4];

		// Check if there's any new data avaiable at all
		if (tmpXInputState.dwPacketNumber == lastPacketNumber[i]){
	//		std::cout<<"\nOld data, skipping";
			continue;
		}
		lastPacketNumber[i] = tmpXInputState.dwPacketNumber;

		// Parse Xbox controller input data
		// http://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.reference[0]input_gamepad%28v=vs.85%29.aspx
		XINPUT_GAMEPAD * input = &tmpXInputState.Gamepad;

		// Get float valuesl.....
#define XINPUT_THUMB_MAX	32768

		gamepadState[i].leftStickX = ((float)input->sThumbLX) / XINPUT_THUMB_MAX;
		gamepadState[i].leftStickY = ((float)input->sThumbLX) / XINPUT_THUMB_MAX;
/*
		Gamepad * gamepad = &gamepadState[i];

		int inputDeviceIndex = GAME_PAD_1 + i;

		// Interpret Game sticks
		// First left Game stick!
		if (gamepad->leftStickX < -0.5f){
			StateMan.ActiveState()->InputProcessor(BEGIN_TURNING_LEFT, inputDeviceIndex);
		}
		else if (gamepad->leftStickX > 0.5f){
			StateMan.ActiveState()->InputProcessor(BEGIN_TURNING_RIGHT, inputDeviceIndex);
		}
		else {
			StateMan.ActiveState()->InputProcessor(STOP_TURNING_LEFT, inputDeviceIndex);
			StateMan.ActiveState()->InputProcessor(STOP_TURNING_RIGHT, inputDeviceIndex);
		}

		// Triggers (left, right)
		// Left trigger
		if (input->bLeftTrigger > 0){
			StateMan.ActiveState()->InputProcessor(BEGIN_BREAKING, inputDeviceIndex);
		}
		// Right trigger
		else if (input->bRightTrigger > 0){
			StateMan.ActiveState()->InputProcessor(BEGIN_ACCELERATION, inputDeviceIndex);
		}
		else
			StateMan.ActiveState()->InputProcessor(STOP_ACCELERATION, inputDeviceIndex);

		/// Button for le boostlur.
		if (input->wButtons & XINPUT_GAMEPAD_A)
			StateMan.ActiveState()->InputProcessor(BEGIN_BOOST, inputDeviceIndex);
		else
			StateMan.ActiveState()->InputProcessor(STOP_BOOST, inputDeviceIndex);

		// Reset position!
		if (input->wButtons & XINPUT_GAMEPAD_Y)
			StateMan.ActiveState()->InputProcessor(RESET_POSITION, inputDeviceIndex);

			*/
	/*
	}
#endif
	*/
