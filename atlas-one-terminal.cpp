/*
    Atlas One terminal 0.5.1
    Created by Marcin Łojek on 4.9.2020

    It only works if the device is connected to COM3 an I have no idea how to change it
*/

#include <iostream>
#include <Windows.h>

using namespace std;

HANDLE hSerial;                                         //Object handling the Serial port
DCB dcbSerialParams = { 0 };                            //Object containing Serial parameters                            
COMMTIMEOUTS timeouts = { 0 };                          //Object handling timeouts on Serial port
char buffer[6];                                         //Serial port buffer   
DWORD dwBytesRead = 0;                                  //Number of bytes read form the Serial port
INPUT input[6];                                         //Objects array used to mock keyboard and mouse input
char lastGest = '2';                                    //Last gest ID for one-per-streak functionality

//Setup functions:
void setupPort() {
    hSerial = CreateFile(L"COM3",
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            cout << "Serial port doesn't exist" << endl;
        }
        cout << "Some other error occured" << endl;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error getting state" << endl;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        cout << "Error setting serial port state" << endl;
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        cout << "Error with setting timeouts" << endl;
    }
}
//Utility functions:
void gestMode(char gestID) {
    /*
        The following function enables user to controll Windows with gests
    */
    switch (gestID) {
    case '3':                                           //Roll left
        if (gestID != lastGest) {
            for (int i = 0; i < 4; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_LWIN;
            input[1].ki.wVk = VK_DOWN;                  //Minimize
            input[2] = input[1];
            input[3] = input[0];
            input[2].ki.dwFlags = KEYEVENTF_KEYUP;
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(4, input, sizeof(INPUT));
        }
        break;
    case '4':                                           //Roll right
        if (gestID != lastGest) {
            for (int i = 0; i < 4; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_LWIN;
            input[1].ki.wVk = VK_UP;                    //Maximize
            input[2] = input[1];
            input[3] = input[0];
            input[2].ki.dwFlags = KEYEVENTF_KEYUP;
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(4, input, sizeof(INPUT));
        }
        break;
    case '5':                                           //Scroll down
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        input[0].mi.mouseData = 180;                    //Scroll down
        SendInput(1, &input[0], sizeof(INPUT));
        break;
    case '6':                                           //Scroll up
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        input[0].mi.mouseData = -180;                   //Scroll up
        SendInput(1, &input[0], sizeof(INPUT));
        break;
    case '7':                                           //Swipe left
        if (gestID != lastGest) {
            for (int i = 0; i < 6; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CONTROL;
            input[1].ki.wVk = VK_LWIN;
            input[2].ki.wVk = VK_RIGHT;                 //Next virtual desktop
            input[3] = input[2];
            input[4] = input[1];
            input[5] = input[0];
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            input[4].ki.dwFlags = KEYEVENTF_KEYUP;
            input[5].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(6, input, sizeof(INPUT));
        }
        break;
    case '8':                                           //Swipe right
        if (gestID != lastGest) {
            for (int i = 0; i < 6; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CONTROL;
            input[1].ki.wVk = VK_LWIN;
            input[2].ki.wVk = VK_LEFT;                  //Previous virtual desktop
            input[3] = input[2];
            input[4] = input[1];
            input[5] = input[0];
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            input[4].ki.dwFlags = KEYEVENTF_KEYUP;
            input[5].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(6, input, sizeof(INPUT));
        }
        break;
    case 'B':                                           //Fist swipe left
        if (gestID != lastGest) {
            for (int i = 0; i < 4; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_MENU;
            input[1].ki.wVk = VK_TAB;                   //Next window (Alt + tab)
            input[2] = input[1];
            input[3] = input[0];
            input[2].ki.dwFlags = KEYEVENTF_KEYUP;
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(4, input, sizeof(INPUT));
        }
        break;
    case 'C':                                           //Fist swipe right
        if (gestID != lastGest) {
            for (int i = 0; i < 6; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_MENU;
            input[1].ki.wVk = VK_SHIFT;
            input[2].ki.wVk = VK_TAB;                   //Previous window (Alt + Shift + tab)
            input[3] = input[2];
            input[4] = input[1];
            input[5] = input[0];
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            input[4].ki.dwFlags = KEYEVENTF_KEYUP;
            input[5].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(6, input, sizeof(INPUT));
        }
        break;
    default:
        break;
    }
    ZeroMemory(&input, sizeof(input));
}
void mediaMode(char gestID) {
    /*
        The following function enables user to controll media with gests
    */
    if (gestID == lastGest)  return;
    input[0].type = INPUT_KEYBOARD;
    switch (gestID) {
    case '3':                                           //Roll left
        input[0].ki.wVk = VK_VOLUME_MUTE;               //Mute
        break;
    case '4':                                           //Roll right
        input[0].ki.wVk = VK_MEDIA_PLAY_PAUSE;          //Play/Pause
        break;
    case '5':                                           //Scroll down
        input[0].ki.wVk = VK_VOLUME_DOWN;               //Vol-
        break;
    case '6':                                           //Scroll up
        input[0].ki.wVk = VK_VOLUME_UP;                 //Vol+
        break;
    case '7':                                           //Swipe left
        input[0].ki.wVk = VK_MEDIA_NEXT_TRACK;          //Next track
        break;
    case '8':                                           //Swipe right
        input[0].ki.wVk = VK_MEDIA_PREV_TRACK;          //Previous track
        break;
    default:
        break;
    }
    input[1] = input[0];
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
    ZeroMemory(&input, sizeof(input));
}
void browserMode(char gestID) {
    /*
        The following function enables user to controll browser with gests
    */
    switch (gestID) {
    case '3':                                           //Roll left
        if (gestID != lastGest) {
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_BROWSER_REFRESH;       //Refresh
            input[1] = input[0];
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, &input[0], sizeof(INPUT));
        }
        break;
    case '4':                                           //Roll right
        if (gestID != lastGest) {
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_BROWSER_STOP;          //Stop
            input[1] = input[0];
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, &input[0], sizeof(INPUT));
        }
        break;
    case '5':                                           //Scroll down
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        input[0].mi.mouseData = 180;                    //Scroll down
        SendInput(1, &input[0], sizeof(INPUT));
        break;
    case '6':                                           //Scroll up
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        input[0].mi.mouseData = -180;                   //Scroll up
        SendInput(1, &input[0], sizeof(INPUT));
        break;
    case '7':                                           //Swipe left
        if (gestID != lastGest) {
            for (int i = 0; i < 4; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CONTROL;
            input[1].ki.wVk = VK_TAB;                   //Next tab (Ctrl + tab)
            input[2] = input[1];
            input[3] = input[0];
            input[2].ki.dwFlags = KEYEVENTF_KEYUP;
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(4, input, sizeof(INPUT));
        }
        break;
    case '8':                                           //Swipe right
        if (gestID != lastGest) {
            for (int i = 0; i < 6; i++)   input[i].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CONTROL;
            input[1].ki.wVk = VK_SHIFT;
            input[2].ki.wVk = VK_TAB;                   //Previous tab (Ctrl + Shift + tab)
            input[3] = input[2];
            input[4] = input[1];
            input[5] = input[0];
            input[3].ki.dwFlags = KEYEVENTF_KEYUP;
            input[4].ki.dwFlags = KEYEVENTF_KEYUP;
            input[5].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(6, input, sizeof(INPUT));
        }
        break;
    case 'B':                                           //Fist swipe left
        if (gestID != lastGest) {
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_BROWSER_FORWARD;       //Forward
            input[1] = input[0];
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, &input[0], sizeof(INPUT));
        }
        break;
    case 'C':                                           //Fist swipe right
        if (gestID != lastGest) {
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_BROWSER_BACK;          //Back
            input[1] = input[0];
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, &input[0], sizeof(INPUT));
        }
        break;
    default:
        break;
    }
    ZeroMemory(&input, sizeof(input));
}
//Main function:
int main()
{
    system("title AtlasW Terminal 0.5.1");
    cout << "WARNING: the following terminal works only with COM3" << endl;
    system("pause");
    system("cls");

    setupPort();
    system("pause");
    system("cls");

    while (true) {
        /*  
            This loop checks if the device is compatible with the terminal        
        */
        if (!ReadFile(hSerial, buffer, 5, &dwBytesRead, NULL)) {
            cout << "Error reading port" << endl << "The Terminal will now shut down" << endl;
            system("pause");
            return 0;
        }

        if (buffer[0] != '\0') {
            cout << buffer << endl;
            if (string(buffer) == "AW051") {
                buffer[0] = 'K';
                buffer[1] = '\0';
                if (!WriteFile(hSerial, buffer, 1, &dwBytesRead, NULL)) {
                    cout << "Error sending data to the serial port" << endl;
                    system("pause");
                    return 0;
                }
                memset(buffer, 0, 6);
                break;
            }
            else {
                cout << "Your device is incompatible with this terminal." << endl;
                cout << "The Terminal will now shut down" << endl;
                system("pause");
                return 0;
            }
        }
    }

    while (true) {
        /*
            This loop reads data from the serial port and calls the according utility function
        */
        if (!ReadFile(hSerial, buffer, 2, &dwBytesRead, NULL)) {
            cout << "Error reading port" << endl << "The Terminal will now shut down" << endl;
            system("pause");
            return 0;
        }

        if ((int)buffer[0] > 47) {                      //ASCII 48 is '0'
            switch (buffer[0])
            {
            case 'A':
                gestMode(buffer[1]);
                break;
            case 'B':
                mediaMode(buffer[1]);
                break;
            case 'C':
                browserMode(buffer[1]);
                break;
            default:
                break;
            }
            cout << buffer[0] << buffer[1] << lastGest << endl;
            lastGest = buffer[1];
        }
        memset(buffer, '\0', 6);
    }

    CloseHandle(hSerial);
}