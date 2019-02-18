#include "stdafx.h"
#include "Scene.h"

//////////////////////////////////////////////////////////////////////////

using namespace CommonLibOgl;
using namespace PhongLightingApp;

//////////////////////////////////////////////////////////////////////////


Scene::Scene(const glm::vec3& backgroundColor, CameraPtr& spCamera, GLuint programId)
	: m_backgroundColor(backgroundColor), m_spCamera(spCamera), m_programId(programId), 
	  m_ambientColor(glm::vec3(1.0f)), 
	  m_ambientIntensity(0.2f),
	  m_light(glm::vec3(1.0f), glm::vec3(1.0f), 1.0f, glm::vec3(0.0f, 0.9f, 0.0f)),
	  m_material(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 128.0f),
	  m_ambientFactor(m_ambientColor * m_ambientIntensity * m_material.m_ambient)
{
}

Scene::~Scene()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (0 != m_index)
	{
		glDeleteBuffers(1, &m_index);
	}

	if (0 != m_vbo)
	{
		glDeleteBuffers(1, &m_vbo);
	}

	if (0 != m_vao)
	{
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_vao);
	}
}

bool Scene::initialize()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_MULTISAMPLE);

	glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, 1.0f);

	if (!initializeContents())
	{
		std::wcerr << L"Failed to initialize scene contents\n";
		return 1;
	}

	updateUniforms();

	return true;
}

bool Scene::initializeContents()
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Set up the vertex buffer.

	std::vector<GLfloat> vertices = {
#if 0
		// A triangle.
		-0.90f, -0.90f, 0.0f,
		0.85f, -0.90f, 0.0f,
		-0.90f, 0.85f, 0.0f
#else
		// A cube.
		 0.9f, -0.9f, -0.9f,
		 0.9f, -0.9f,  0.9f,
		-0.9f, -0.9f,  0.9f, 
		-0.9f, -0.9f, -0.9f,
		 0.9f,  0.9f, -0.9f,
		 0.9f,  0.9f,  0.9f,
		-0.9f,  0.9f,  0.9f, 
		-0.9f,  0.9f, -0.9f
#endif
	};

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

	// Fill in the vertex position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	// Set up the index buffer.

	std::vector<GLuint> indices = {
#if 0
		// A triangle.
		0, 1, 2 
#else
		// A cube.
		1, 3, 0,
		7, 5, 4,
		4, 1, 0,
		5, 2, 1,
		2, 7, 3,
		0, 7, 4,
		1, 2, 3,
		7, 6, 5,
		4, 5, 1,
		5, 6, 2,
		2, 6, 7,
		0, 3, 7
#endif
	};

	m_indexCount = indices.size();

	glGenBuffers(1, &m_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Set up the normal buffer.
#if 1
	std::vector<GLfloat> normals = {
		 0.0f, -1.0f,  0.0f, 
		 0.0f,  1.0f,  0.0f,
		 1.0f, -0.0f,  0.0f,
		 0.0f, -0.0f,  1.0f, 
		-1.0f, -0.0f, -0.0f, 
		 0.0f,  0.0f, -1.0f
	};

	glGenBuffers(1, &m_normal);
	glBindBuffer(GL_ARRAY_BUFFER, m_normal);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]), &normals[0], GL_STATIC_DRAW);

	// Fill in the normal attribute.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
#endif

	updateUniforms();

	return true;
}

void Scene::updateUniforms() const
{
	ATLASSERT(m_programId);

	glUseProgram(m_programId);

	// TODO:
	//     1) comment out the uniforms you don't need.
	//     2) correct the uniform locations if required.

	// Pre-calculated ambient factor for the material.
	glUniform3fv(0, 1, glm::value_ptr(m_ambientFactor));

	// Light properties.
	glUniform3fv(1, 1, glm::value_ptr(m_light.m_diffuse));
	glUniform3fv(2, 1, glm::value_ptr(m_light.m_specular));
	glUniform1f(3, m_light.m_intensity);
	glUniform3fv(4, 1, glm::value_ptr(m_light.m_position));

	// Material properties.
	glUniform3fv(5, 1, glm::value_ptr(m_material.m_diffuse));
	glUniform3fv(6, 1, glm::value_ptr(m_material.m_specular));
	glUniform1f(7, m_material.m_shininess);

#if 1
	glm::mat4 modelView = m_spCamera->getModelViewMatrix();

	glUniformMatrix4fv(8, 1, GL_FALSE, glm::value_ptr(modelView));

	glUniformMatrix4fv(9, 1, GL_FALSE, glm::value_ptr(m_spCamera->getViewMatrix()));

	glUniformMatrix4fv(10, 1, GL_FALSE, glm::value_ptr(m_spCamera->getProjectionMatrix()));

#if 0
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_spCamera->getModelViewProjectionMatrix()));

	// WARNING: we are using the fact that there are no non-uniform scaling. If this will change, use the entire 4x4 matrix.
	glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(modelView)));
	//glm::mat3 normal = glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]));

	glUniformMatrix3fv(2, 1, GL_FALSE, glm::value_ptr(normal));
#endif

#endif

	glUseProgram(0);
}

void Scene::translateCamera(const glm::vec3& diff)
{
	m_spCamera->translate(diff);

	updateUniforms();
}

void Scene::rotateCamera(const glm::vec3& degrees)
{
	m_spCamera->rotate(degrees);

	updateUniforms();
}

GLfloat Scene::getCameraScale() const
{
	return m_spCamera->getScale();
}

void Scene::scaleCamera(GLfloat amount)
{
	m_spCamera->scale(amount);

	updateUniforms();
}

void Scene::resize(GLfloat aspectRatio)
{
	m_spCamera->resize(aspectRatio);

	updateUniforms();
}

void Scene::render() const
{
	ATLASSERT(m_programId);

	updateUniforms();

	glUseProgram(m_programId);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index);

	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}
