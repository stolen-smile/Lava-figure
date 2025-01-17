// lab3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma region prototypesAndVariables
//расширения
GLuint screenWidth = 800, screenHeight = 600;
//проттотипы функции
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
//глобальные переменные
bool keys[1024];
GLfloat lastX = screenWidth / 2, lastY = screenHeight/2;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
//камера
Camera camera(glm::vec3(2.5f, 4.0f, 4.0f));
#pragma endregion
GLfloat triangle[]={
	//передняя грань (z+)
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
};
GLfloat quadrate[] = {
	//передняя грань (z+)
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
};

int main()
{
	// Инициализация GLFW
	glfwInit();
	//Настройка GLFW
	//Задается минимальная требуемая версия OpenGL. 
	//Мажорная 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//Минорная
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Установка профайла для которого создается контекст
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Выключение возможности изменения размера окна
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	//создаем объект окна

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Установка выбраных функций обратной связи
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//прячич курсор
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		system("pause");
		return -1;
	}
	// Определяем размеры области просмотра
	glViewport(0, 0, screenWidth, screenHeight);
	glEnable(GL_DEPTH_TEST);
	//загружаем шейдеры
	Shader ourShader("Shaders/verticies.vert", "Shaders/fragments.frag");

#pragma region VBO_&_VAO
	//создание VBO,VAO
	GLuint VBO, VAO_T,VAO_Q;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO_T);
	glGenVertexArrays(1, &VAO_Q);
	
	//Привязываем VAO
	glBindVertexArray(VAO_T);
	//мы привязываем GL_ARRAY_BUFFER к нашему буферу
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//копирования вершинных данных в этот буфер
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//Отвязываем VAO_T
	glBindVertexArray(0);
	//----------------VAO_Q---------------
	glBindVertexArray(VAO_Q);
	//копирования вершинных данных в этот буфер
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadrate), quadrate, GL_STATIC_DRAW);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//Отвязываем VAO_Q
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#pragma endregion

#pragma region Textures
	//индификаторы текстур
	GLuint texture1;
	GLuint texture2;
	//генерация первой текстуры
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	//Устанавливаем настройки фильтрации и преобразований (на текущей текстуре)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//Устанавливаем настройки фильтрации текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//переменные для размера картинки
	int width, height;
	// Загружаем и генерируем текстуру
	unsigned char* image = SOIL_load_image("Textures/magma.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	//генерация мипмапов
	glGenerateMipmap(GL_TEXTURE_2D);
	//освобождение памяти от картинки
	SOIL_free_image_data(image);
	//отвязывание текстуры
	glBindTexture(GL_TEXTURE_2D, 0);

#pragma endregion
	while (!glfwWindowShouldClose(window))
	{
		//Вычисляем время между кадрами
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Do_Movement();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//рисуем первый треугольник
		ourShader.Use();

		//Связываем текстуры с текстырными сущностями
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glUniform1i(glGetUniformLocation(ourShader.Program, "ourTexture1"), 0);
		
		// Трансформации камеры
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection;
		projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 1000.0f);
		// Gполучаем локации глобальных переменных в шейдерах
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
		// Передаем матрицы в шейдера
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO_Q);
		glm::mat4 model;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
#pragma region backWall
		//задняя стенка	
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, 0));	
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 1, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 2, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 3; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 3, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
#pragma endregion
#pragma region FrontWall
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 1, -1));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 2, -1));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 3; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 3, -1));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
#pragma endregion
#pragma region BehindWall
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -4));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i+3, 0, -4));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(2, 0, -3));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
#pragma endregion	
#pragma region floor
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -1));
			model = glm::rotate(model, 90.0f, glm::vec3(-1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -2));
			model = glm::rotate(model, 90.0f, glm::vec3(-1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -3));
			model = glm::rotate(model, 90.0f, glm::vec3(-1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -4));
			model = glm::rotate(model, 90.0f, glm::vec3(-1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i+3, 0, -4));
			model = glm::rotate(model, 90.0f, glm::vec3(-1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
#pragma endregion
#pragma region secondFloor
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -2));
			model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 5; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -3));
			model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 0, -4));
			model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 0; i < 2; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i + 3, 0, -4));
			model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
#pragma endregion
#pragma region roof		
		for (int i = 0; i < 3; i++)
		{
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(i, 3, -1));
			model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		//sloping roof
		//first panel
		model = glm::mat4();
		model = glm::translate(model, glm::vec3( 2.49, 2.8, -1));
		model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
		model = glm::rotate(model, 45.0f, glm::vec3(0, -1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		//second panel
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(3.19, 2.09, -1));
		model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
		model = glm::rotate(model, 45.0f, glm::vec3(0, -1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		//third panel
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(3.79, 1.5, -1));
		model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
		model = glm::rotate(model, 45.0f, glm::vec3(0, -1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36


		//for (int i = 0; i < 3; i++)
		//{
		//	model = glm::mat4();
		//	model = glm::translate(model, glm::vec3(i+3, 3, -1));
		//	model = glm::rotate(model, 90.0f, glm::vec3(1, 0, 0));
		//	model = glm::rotate(model, 45.0f, glm::vec3(0, -1, 0));
		//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//	glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		//}

#pragma endregion
#pragma region leftSide
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3(i+1, 0, 0));
			
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 1; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3( 1, i, 0));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		model = glm::mat4();
		model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(1, 1, 5));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
#pragma endregion
#pragma region rightSide
		for (int i = 0; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3(i + 1, 0, 5));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
		for (int i = 1; i < 4; i++)
		{
			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
			model = glm::translate(model, glm::vec3(1, i, 0));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
		}
#pragma endregion
#pragma region insideWall
		model = glm::mat4();
		model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(4, 0, 2));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36

		model = glm::mat4();
		model = glm::rotate(model, 90.0f, glm::vec3(0, 1, 0));
		model = glm::translate(model, glm::vec3(4, 0, 3));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);//было 36
#pragma endregion

		glBindVertexArray(0);
		glBindVertexArray(VAO_T);
#pragma region triangleWall
			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 0, -1));
			model = glm::translate(model, glm::vec3(-3, 3, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 3);//было 36

			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 0, -1));
			model = glm::translate(model, glm::vec3(-3, 3, -1));//
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 3);//было 36

			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 0, -1));
			model = glm::translate(model, glm::vec3(-2, 4, 0));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 3);//было 36

			model = glm::mat4();
			model = glm::rotate(model, 90.0f, glm::vec3(0, 0, -1));
			model = glm::translate(model, glm::vec3(-2, 4, -1));//
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, 3);//было 36
#pragma endregion
		glBindVertexArray(0);
		// Swap the buffers
		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO_T);
	glDeleteVertexArrays(1, &VAO_Q);
	
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
    return 0;
}
// Moves/alters the camera positions based on user input
//Функция передвигает позицию камеры основываясь на юзерских входных данных
void Do_Movement()
{
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}
//Функция вызывается всякий раз, когда клавиша нажимается/отпускается
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Действие обратное, поскольку y-координаты идут снизу влево

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
