#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>

const POINT kLossPixel     = {794, 146};             // Loss pixel (black)
const POINT kWinPixel      = {790, 139};             // Win pixel (black)
const COLORREF kTargetColor = RGB(0, 0, 0);          // Black for win/loss
const POINT kResetButton   = {798, 139};             // Reset button

const COLORREF kUnclickedColor = RGB(189, 189, 189); // Color of squares to click

bool ColorsMatch(COLORREF c1, COLORREF c2) {
    return GetRValue(c1) == GetRValue(c2) &&
           GetGValue(c1) == GetGValue(c2) &&
           GetBValue(c1) == GetBValue(c2);
}

void ClickAt(int x, int y) {
    SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(10);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

std::vector<POINT> LoadClickCoordinates(const std::string& filename) {
    std::vector<POINT> coordinates;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int x, y;
        if (iss >> x >> y) {
            coordinates.push_back({x, y});
        }
    }

    return coordinates;
}

int main() {
    std::cout << "Minesweeper Clicker Bot started. Press ESC to stop.\n";

    std::vector<POINT> clickTargets = LoadClickCoordinates("coords.txt");
    if (clickTargets.empty()) {
        std::cerr << "Error: No coordinates loaded from coords.txt.\n";
        return 1;
    }

    HDC screenDC = GetDC(NULL);
    srand(static_cast<unsigned>(time(0)));

    while (true) {
        // Exit on ESC key press
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            std::cout << "ESC pressed. Exiting program.\n";
            break;
        }

        COLORREF lossColor = GetPixel(screenDC, kLossPixel.x, kLossPixel.y);
        COLORREF winColor = GetPixel(screenDC, kWinPixel.x, kWinPixel.y);

        if (ColorsMatch(winColor, kTargetColor)) {
            std::cout << "Win detected. Bot will wait until stopped manually.\n";
            // Wait indefinitely but check ESC frequently
            while (true) {
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                    std::cout << "ESC pressed. Exiting program.\n";
                    goto end_program;
                }
                Sleep(50);
            }
        }

        if (ColorsMatch(lossColor, kTargetColor)) {
            std::cout << "Loss detected. Resetting board.\n";
            ClickAt(kResetButton.x, kResetButton.y);
            Sleep(600);
            continue;
        }

        // Filter clickable squares by checking color of each coordinate
        std::vector<POINT> availableSquares;
        for (const auto& p : clickTargets) {
            COLORREF pixelColor = GetPixel(screenDC, p.x, p.y);
            if (ColorsMatch(pixelColor, kUnclickedColor)) {
                availableSquares.push_back(p);
            }
        }

        if (availableSquares.empty()) {
            std::cout << "No unclicked squares left. Waiting...\n";
            Sleep(500);
            continue;
        }

        int index = rand() % availableSquares.size();
        POINT p = availableSquares[index];
        ClickAt(p.x, p.y);
        std::cout << "Clicked at: (" << p.x << ", " << p.y << ")\n";

        Sleep(50);  // Faster clicking: 50 ms delay (~20 clicks/sec)
    }

end_program:
    ReleaseDC(NULL, screenDC);
    return 0;
}