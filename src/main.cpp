float prevWidth;
float prevHeight;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return;
    prevWidth = width;
    prevHeight = height;
    unsigned int ubo = *(unsigned int*)glfwGetWindowUserPointer(window);

    setProjectionMatrix(ubo, width, height);

    glViewport(0, 0, width, height);
}

void drawLoop(GLFWwindow* window) {
    double currentTime = glfwGetTime();
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwGetCursorPos(window, &cursorX, &cursorY);

        cursorX /= prevHeight / 2;
        cursorX -= prevWidth / prevHeight;
        cursorY /= -prevHeight / 2;
        cursorY += 1;

        double newTime = glfwGetTime();
        double delta = newTime - currentTime;
        currentTime = newTime;

        glClear(GL_COLOR_BUFFER_BIT);

        if (gameState == State::EXIT) {
            glfwSetWindowShouldClose(window, 1);
        } else if (gameState == State::JOIN) {
            buffer[0] = (char)ClientMessage::Join;

            if (!net::socketSend(&sock, buffer, 1, &serverEndpoint)) {
                printf("join message failed\n");
                gameState = State::MENU;
            } else {
                gameState = State::PLAY;
            }
        } else if (gameState == State::LEAVE) {
            buffer[0] = (unsigned char)ClientMessage::Leave;
            int bytesWritten = 1;
            memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
            net::socketSend(&sock, buffer, bytesWritten, &serverEndpoint);
            gameState = State::MENU;
        }

        prevInput = input;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    buffer[0] = (unsigned char)ClientMessage::Leave;
    int bytesWritten = 1;
    memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
    net::socketSend(&sock, buffer, bytesWritten, &serverEndpoint);

    glDeleteProgram(playerShaderProgram);
    glDeleteProgram(ballShaderProgram);
    glDeleteProgram(textShaderProgram);
    glfwTerminate();
    return;
}

void setVSync(bool sync) {
    typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALPROC)(int);
    PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

    const char* extensions = (char*)glGetString(GL_EXTENSIONS);

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(sync);
}

int main() {
    if (!glfwInit()) {
        printf("GFLW failed to initialize.");
        return -1;
    }
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    prevWidth = mode->width;
    prevHeight = mode->height;

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Poooong", monitor, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSwapInterval(1);

    render::RenderData renderData = render::init(window);

    drawLoop(window);

    return 0;
}