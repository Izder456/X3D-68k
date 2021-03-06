#include "system/Clock.hpp"
#include "render/Screen.hpp"
#include "Console.hpp"
#include "ConsoleRenderer.hpp"
#include "render/Font.hpp"
#include "engine/EngineContext.hpp"

const Duration toggleTime = Duration::fromMilliseconds(250);
const Duration cursorBlinkTime = Duration::fromMilliseconds(250);

void ConsoleRenderer::show()
{
    consoleToggleTime = Clock::getTicks();
    openState = X_CONSOLE_STATE_OPENING;
}

void ConsoleRenderer::hide()
{
    consoleToggleTime = Clock::getTicks();
    openState = X_CONSOLE_STATE_CLOSING;
}

void ConsoleRenderer::handleCursorBlinking()
{
    Time currentTime = Clock::getTicks();
    
    if(currentTime - lastCursorBlink >= cursorBlinkTime)
    {
        showCursor = !showCursor;
        lastCursorBlink = currentTime;
    }
}

void ConsoleRenderer::handleOpeningAnimation()
{
    Duration timePassed = Clock::getTicks() - consoleToggleTime;
    int consoleHeight = getConsoleHeight();

    renderYOffset = (-consoleHeight * ((toggleTime - timePassed) / toggleTime)).toInt();
    
    if(renderYOffset >= 0)
    {
        renderYOffset = 0;
        openState = X_CONSOLE_STATE_OPEN;
    }
}

void ConsoleRenderer::handleClosingAnimation()
{
    Duration timePassed = Clock::getTicks() - consoleToggleTime;
    int consoleHeight = getConsoleHeight();

    renderYOffset = -(consoleHeight * (timePassed / toggleTime)).toInt();
    
    if(renderYOffset <= -consoleHeight)
    {
        openState = X_CONSOLE_STATE_CLOSED;
    }
}

void ConsoleRenderer::handleStateAnimations()
{
    if(openState == X_CONSOLE_STATE_OPENING)
    {
        handleOpeningAnimation();
    }
    else if(openState == X_CONSOLE_STATE_CLOSING)
    {
        handleClosingAnimation();
    }
}

void ConsoleRenderer::renderOutput()
{
    for(int i = 0; i < console.size.y; ++i)
    {
        screen.canvas.drawStr(
            console.getLine(i),
            font,
            {
                0,
                getLineY(i)
            });
    }
}

int ConsoleRenderer::getLineY(int lineNumber)
{
    return lineNumber * console.font->getH() + renderYOffset;
}

int ConsoleRenderer::getConsoleHeight()
{
    return console.size.y * font.getH();
}


void ConsoleRenderer::renderBackground()
{
    const X_Palette* palette = screen.palette;

    X_Color backgroundColor = palette->black;
    X_Color lineColor = palette->darkRed;
    
    Vec2 topLeft =
    {
        0,
        getLineY(0)
    };

    Vec2 bottomRight =
    {
        screen.getW() - 1,
        getLineY(console.size.y)
    };
    
    screen.canvas.fillRect(topLeft, bottomRight, backgroundColor);
    
    Vec2 bottomLeft = Vec2(0, bottomRight.y);
    screen.canvas.drawLine(bottomLeft, bottomRight, lineColor);
}

char* ConsoleRenderer::getStartOfScrolledInput()
{
    char* input = console.input;
    int inputLenghtIncludingCursor = console.inputPos + 3; 
    
    if(inputLenghtIncludingCursor >= console.size.x)
    {
        int charsToScrollHorizontallyBy = inputLenghtIncludingCursor - console.size.x;
        input += charsToScrollHorizontallyBy;
    }
    
    return input;
}

int ConsoleRenderer::getNextEmptyLine()
{
    return console.cursor.y + (console.cursor.x == 0 ? 0 : 1);
}

void ConsoleRenderer::removeCursorFromInputBuf()
{
    console.input[console.inputPos] = '\0';
}

void ConsoleRenderer::addCursorToInputBuf()
{
    const unsigned char CURSOR_CHAR = 11;
    console.input[console.inputPos] = CURSOR_CHAR;
    console.input[console.inputPos + 1] = '\0';
}

void ConsoleRenderer::renderInput()
{
    handleCursorBlinking();
    
    if(showCursor)
    {
        addCursorToInputBuf();
    }
        
    char* scrolledInput = getStartOfScrolledInput();
    int nextEmptyLine = getNextEmptyLine();
    int inputLineY = getLineY(nextEmptyLine);
    const int CHARS_IN_CURSOR = 2;

    screen.canvas.drawChar(']', font, { 0, inputLineY });
    
    screen.canvas.drawStr(
        scrolledInput,
        font,
        {
            font.getW() * CHARS_IN_CURSOR,
            inputLineY
        });
    
    removeCursorFromInputBuf();
}

void ConsoleRenderer::render()
{
    handleStateAnimations();
    
    if(!isVisible())
    {
        return;
    }
    
    renderBackground();
    renderOutput();
    renderInput();
}

