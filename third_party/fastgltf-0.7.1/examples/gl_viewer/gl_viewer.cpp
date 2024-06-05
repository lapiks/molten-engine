/*
 * Copyright (C) 2022 - 2024 spnda
 * This file is part of fastgltf <https://github.com/spnda/fastgltf>.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <chrono>
#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// All headers not in the root directory require this
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

constexpr std::string_view vertexShaderSource = R"(
    #version 460 core

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 inTexCoord;

    uniform mat4 modelMatrix;
    uniform mat4 viewProjectionMatrix;

    out vec2 texCoord;

    void main() {
        gl_Position = viewProjectionMatrix * modelMatrix * vec4(position, 1.0);
        texCoord = inTexCoord;
    }
)";

constexpr std::string_view fragmentShaderSource = R"(
    #version 460 core

    in vec2 texCoord;
    out vec4 finalColor;

	uniform vec2 uvOffset, uvScale;
	uniform float uvRotation;

    const uint HAS_BASE_COLOR_TEXTURE = 1;

    layout(location = 0) uniform sampler2D albedoTexture;
    layout(location = 0, std140) uniform MaterialUniforms {
        vec4 baseColorFactor;
        float alphaCutoff;
        uint flags;
    } material;

    float rand(vec2 co){
        return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
    }

	vec2 transformUv(vec2 uv) {
		mat2 rotationMat = mat2(
			cos(uvRotation), -sin(uvRotation),
		   	sin(uvRotation), cos(uvRotation)
		);
		return rotationMat * uv * uvScale + uvOffset;
	}

    void main() {
        vec4 color = material.baseColorFactor;
        if ((material.flags & HAS_BASE_COLOR_TEXTURE) == HAS_BASE_COLOR_TEXTURE) {
            color *= texture(albedoTexture, transformUv(texCoord));
        }
        float factor = (rand(gl_FragCoord.xy) - 0.5) / 8;
        if (color.a < material.alphaCutoff + factor)
            discard;
        finalColor = color;
    }
)";

void glMessageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) {
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        std::cerr << message << '\n';
    } else {
        std::cout << message << '\n';
    }
}

bool checkGlCompileErrors(GLuint shader) {
    GLint success;
    constexpr int length = 1024;
    std::string log;
    log.resize(length);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::cout << "Shader compilation error: " << "\n"
                  << log << "\n -- --------------------------------------------------- -- " << '\n';
        return false;
    }
    return true;
}

bool checkGlLinkErrors(GLuint target) {
    GLint success;
    constexpr int length = 1024;
    std::string log;
    log.resize(length);
    glGetProgramiv(target, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        glGetShaderInfoLog(target, length, nullptr, log.data());
        std::cout << "Shader program linking error: " << "\n"
                  << log << "\n -- --------------------------------------------------- -- " << '\n';
        return false;
    }
    return true;
}

struct IndirectDrawCommand {
	std::uint32_t count;
	std::uint32_t instanceCount;
	std::uint32_t firstIndex;
	std::int32_t baseVertex;
	std::uint32_t baseInstance;
};

struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
};

struct Primitive {
    IndirectDrawCommand draw;
    GLenum primitiveType;
    GLenum indexType;
    GLuint vertexArray;

	GLuint vertexBuffer;
	GLuint indexBuffer;

    std::size_t materialUniformsIndex;
    GLuint albedoTexture;
};

struct Mesh {
    GLuint drawsBuffer;
    std::vector<Primitive> primitives;
};

struct Texture {
    GLuint texture;
};

enum MaterialUniformFlags : std::uint32_t {
    None = 0 << 0,
    HasBaseColorTexture = 1 << 0,
};

struct MaterialUniforms {
    glm::fvec4 baseColorFactor;
    float alphaCutoff;
	std::uint32_t flags;

	glm::vec2 padding;
};

struct Viewer {
    fastgltf::Asset asset;

    std::vector<GLuint> bufferAllocations;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
	std::vector<glm::mat4> cameras;

    std::vector<MaterialUniforms> materials;
    std::vector<GLuint> materialBuffers;

	GLint uvOffsetUniform = GL_NONE;
	GLint uvScaleUniform = GL_NONE;
	GLint uvRotationUniform = GL_NONE;

	glm::ivec2 windowDimensions = glm::ivec2(0);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    GLint viewProjectionMatrixUniform = GL_NONE;
    GLint modelMatrixUniform = GL_NONE;

    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    glm::vec3 accelerationVector = glm::vec3(0.0f);
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::dvec2 lastCursorPosition = glm::dvec2(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
    float yaw = -90.0f;
    float pitch = 0.0f;
    bool firstMouse = true;

	std::size_t sceneIndex = 0;
	std::size_t materialVariant = 0;
	fastgltf::Optional<std::size_t> cameraIndex = std::nullopt;
};

void updateCameraMatrix(Viewer* viewer) {
    glm::mat4 viewProjection = viewer->projectionMatrix * viewer->viewMatrix;
    glUniformMatrix4fv(viewer->viewProjectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewProjection));
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    void* ptr = glfwGetWindowUserPointer(window);
    auto* viewer = static_cast<Viewer*>(ptr);

	viewer->windowDimensions = { width, height };

    glViewport(0, 0, width, height);
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    void* ptr = glfwGetWindowUserPointer(window);
    auto* viewer = static_cast<Viewer*>(ptr);

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
	if (state != GLFW_PRESS) {
		viewer->lastCursorPosition = { xpos, ypos };
		return;
	}

    if (viewer->firstMouse) {
        viewer->lastCursorPosition = { xpos, ypos };
        viewer->firstMouse = false;
    }

    auto offset = glm::vec2(xpos - viewer->lastCursorPosition.x, viewer->lastCursorPosition.y - ypos);
    viewer->lastCursorPosition = { xpos, ypos };
    offset *= 0.1f;

    viewer->yaw   += offset.x;
    viewer->pitch += offset.y;
    viewer->pitch = glm::clamp(viewer->pitch, -89.0f, 89.0f);

    auto& direction = viewer->direction;
    direction.x = cos(glm::radians(viewer->yaw)) * cos(glm::radians(viewer->pitch));
    direction.y = sin(glm::radians(viewer->pitch));
    direction.z = sin(glm::radians(viewer->yaw)) * cos(glm::radians(viewer->pitch));
    direction = glm::normalize(direction);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    void* ptr = glfwGetWindowUserPointer(window);
    auto* viewer = static_cast<Viewer*>(ptr);
    static constexpr auto cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    auto& acceleration = viewer->accelerationVector;
    switch (key) {
        case GLFW_KEY_W:
            acceleration += viewer->direction;
            break;
        case GLFW_KEY_S:
            acceleration -= viewer->direction;
            break;
        case GLFW_KEY_D:
            acceleration += glm::normalize(glm::cross(viewer->direction, cameraUp));
            break;
        case GLFW_KEY_A:
            acceleration -= glm::normalize(glm::cross(viewer->direction, cameraUp));
            break;
        default:
            break;
    }
}

glm::mat4 getTransformMatrix(const fastgltf::Node& node, glm::mat4x4& base) {
    /** Both a matrix and TRS values are not allowed
     * to exist at the same time according to the spec */
    if (const auto* pMatrix = std::get_if<fastgltf::Node::TransformMatrix>(&node.transform)) {
        return base * glm::mat4x4(glm::make_mat4x4(pMatrix->data()));
    }

	if (const auto* pTransform = std::get_if<fastgltf::TRS>(&node.transform)) {
		// Warning: The quaternion to mat4x4 conversion here is not correct with all versions of glm.
		// glTF provides the quaternion as (x, y, z, w), which is the same layout glm used up to version 0.9.9.8.
		// However, with commit 59ddeb7 (May 2021) the default order was changed to (w, x, y, z).
		// You could either define GLM_FORCE_QUAT_DATA_XYZW to return to the old layout,
		// or you could use the recently added static factory constructor glm::quat::wxyz(w, x, y, z),
		// which guarantees the parameter order.
        return base
            * glm::translate(glm::mat4(1.0f), glm::make_vec3(pTransform->translation.data()))
            * glm::toMat4(glm::make_quat(pTransform->rotation.data()))
            * glm::scale(glm::mat4(1.0f), glm::make_vec3(pTransform->scale.data()));
    }

	return base;
}

bool loadGltf(Viewer* viewer, std::string_view cPath) {
	if (!std::filesystem::exists(cPath)) {
		std::cout << "Failed to find " << cPath << '\n';
		return false;
	}

    std::cout << "Loading " << cPath << '\n';

    // Parse the glTF file and get the constructed asset
    {
		static constexpr auto supportedExtensions =
			fastgltf::Extensions::KHR_mesh_quantization |
			fastgltf::Extensions::KHR_texture_transform |
			fastgltf::Extensions::KHR_materials_variants;

        fastgltf::Parser parser(supportedExtensions);

        auto path = std::filesystem::path{cPath};

        constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadGLBBuffers |
            fastgltf::Options::LoadExternalBuffers |
            fastgltf::Options::LoadExternalImages |
			fastgltf::Options::GenerateMeshIndices;

        fastgltf::GltfDataBuffer data;
        data.loadFromFile(path);

        auto asset = parser.loadGltf(&data, path.parent_path(), gltfOptions);
        if (asset.error() != fastgltf::Error::None) {
            std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error()) << '\n';
            return false;
        }

        viewer->asset = std::move(asset.get());
    }

    return true;
}

bool loadMesh(Viewer* viewer, fastgltf::Mesh& mesh) {
    auto& asset = viewer->asset;
    Mesh outMesh = {};
    outMesh.primitives.resize(mesh.primitives.size());

    for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it) {
		auto* positionIt = it->findAttribute("POSITION");
		assert(positionIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute.
		assert(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices

        // Generate the VAO
        GLuint vao = GL_NONE;
        glCreateVertexArrays(1, &vao);

		std::size_t baseColorTexcoordIndex = 0;

        // Get the output primitive
        auto index = std::distance(mesh.primitives.begin(), it);
        auto& primitive = outMesh.primitives[index];
        primitive.primitiveType = fastgltf::to_underlying(it->type);
        primitive.vertexArray = vao;
        if (it->materialIndex.has_value()) {
            primitive.materialUniformsIndex = it->materialIndex.value() + 1; // Adjust for default material
            auto& material = viewer->asset.materials[it->materialIndex.value()];

			auto& baseColorTexture = material.pbrData.baseColorTexture;
            if (baseColorTexture.has_value()) {
                auto& texture = viewer->asset.textures[baseColorTexture->textureIndex];
				if (!texture.imageIndex.has_value())
					return false;
                primitive.albedoTexture = viewer->textures[texture.imageIndex.value()].texture;

				if (baseColorTexture->transform && baseColorTexture->transform->texCoordIndex.has_value()) {
					baseColorTexcoordIndex = baseColorTexture->transform->texCoordIndex.value();
				} else {
					baseColorTexcoordIndex = material.pbrData.baseColorTexture->texCoordIndex;
				}
            }
        } else {
			primitive.materialUniformsIndex = 0;
		}

        {
            // Position
            auto& positionAccessor = asset.accessors[positionIt->second];
            if (!positionAccessor.bufferViewIndex.has_value())
                continue;

			// Create the vertex buffer for this primitive, and use the accessor tools to copy directly into the mapped buffer.
			glCreateBuffers(1, &primitive.vertexBuffer);
			glNamedBufferData(primitive.vertexBuffer, positionAccessor.count * sizeof(Vertex), nullptr, GL_STATIC_DRAW);
			auto* vertices = static_cast<Vertex*>(glMapNamedBuffer(primitive.vertexBuffer, GL_WRITE_ONLY));
			fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, positionAccessor, [&](glm::vec3 pos, std::size_t idx) {
				vertices[idx].position = pos;
				vertices[idx].uv = glm::vec2();
			});
			glUnmapNamedBuffer(primitive.vertexBuffer);

            glEnableVertexArrayAttrib(vao, 0);
            glVertexArrayAttribFormat(vao, 0,
                                      3, GL_FLOAT,
                                      GL_FALSE, 0);
            glVertexArrayAttribBinding(vao, 0, 0);

			glVertexArrayVertexBuffer(vao, 0, primitive.vertexBuffer,
									  0, sizeof(Vertex));
        }

		auto texcoordAttribute = std::string("TEXCOORD_") + std::to_string(baseColorTexcoordIndex);
        if (const auto* texcoord = it->findAttribute(texcoordAttribute); texcoord != it->attributes.end()) {
            // Tex coord
			auto& texCoordAccessor = asset.accessors[texcoord->second];
            if (!texCoordAccessor.bufferViewIndex.has_value())
                continue;

			auto* vertices = static_cast<Vertex*>(glMapNamedBuffer(primitive.vertexBuffer, GL_WRITE_ONLY));
			fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, texCoordAccessor, [&](glm::vec2 uv, std::size_t idx) {
				vertices[idx].uv = uv;
			});
			glUnmapNamedBuffer(primitive.vertexBuffer);

			glEnableVertexArrayAttrib(vao, 1);
            glVertexArrayAttribFormat(vao, 1,
									  2, GL_FLOAT,
                                      GL_FALSE, 0);
            glVertexArrayAttribBinding(vao, 1, 1);

			glVertexArrayVertexBuffer(vao, 1, primitive.vertexBuffer,
									  offsetof(Vertex, uv), sizeof(Vertex));
        }

        // Generate the indirect draw command
        auto& draw = primitive.draw;
        draw.instanceCount = 1;
        draw.baseInstance = 0;
        draw.baseVertex = 0;
		draw.firstIndex = 0;

        auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];
        if (!indexAccessor.bufferViewIndex.has_value())
            return false;
        draw.count = static_cast<std::uint32_t>(indexAccessor.count);

		// Create the index buffer and copy 32-bit indices into it.
		glCreateBuffers(1, &primitive.indexBuffer);
		glNamedBufferData(primitive.indexBuffer, static_cast<GLsizeiptr>(indexAccessor.count * sizeof(std::uint32_t)), nullptr, GL_STATIC_DRAW);
		auto* indices = static_cast<std::uint32_t*>(glMapNamedBuffer(primitive.indexBuffer, GL_WRITE_ONLY));
		fastgltf::copyFromAccessor<std::uint32_t>(asset, indexAccessor, indices);
		glUnmapNamedBuffer(primitive.indexBuffer);

        primitive.indexType = GL_UNSIGNED_INT;
        glVertexArrayElementBuffer(vao, primitive.indexBuffer);
    }

    // Create the buffer holding all of our primitive structs.
    glCreateBuffers(1, &outMesh.drawsBuffer);
    glNamedBufferData(outMesh.drawsBuffer, static_cast<GLsizeiptr>(outMesh.primitives.size() * sizeof(Primitive)),
                      outMesh.primitives.data(), GL_STATIC_DRAW);

    viewer->meshes.emplace_back(outMesh);

    return true;
}

bool loadImage(Viewer* viewer, fastgltf::Image& image) {
    auto getLevelCount = [](int width, int height) -> GLsizei {
        return static_cast<GLsizei>(1 + floor(log2(width > height ? width : height)));
    };

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    std::visit(fastgltf::visitor {
        [](auto& arg) {},
        [&](fastgltf::sources::URI& filePath) {
            assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
            assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.
            int width, height, nrChannels;

            const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
            glTextureStorage2D(texture, getLevelCount(width, height), GL_RGBA8, width, height);
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        },
        [&](fastgltf::sources::Array& vector) {
            int width, height, nrChannels;
            unsigned char *data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
            glTextureStorage2D(texture, getLevelCount(width, height), GL_RGBA8, width, height);
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        },
        [&](fastgltf::sources::BufferView& view) {
            auto& bufferView = viewer->asset.bufferViews[view.bufferViewIndex];
            auto& buffer = viewer->asset.buffers[bufferView.bufferIndex];
            // Yes, we've already loaded every buffer into some GL buffer. However, with GL it's simpler
            // to just copy the buffer data again for the texture. Besides, this is just an example.
            std::visit(fastgltf::visitor {
                // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning
                // all buffers are already loaded into a vector.
                [](auto& arg) {},
                [&](fastgltf::sources::Array& vector) {
                    int width, height, nrChannels;
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset, static_cast<int>(bufferView.byteLength), &width, &height, &nrChannels, 4);
                    glTextureStorage2D(texture, getLevelCount(width, height), GL_RGBA8, width, height);
                    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    stbi_image_free(data);
                }
            }, buffer.data);
        },
    }, image.data);

    glGenerateTextureMipmap(texture);

    viewer->textures.emplace_back(Texture { texture });
    return true;
}

bool loadMaterial(Viewer* viewer, fastgltf::Material& material) {
    MaterialUniforms uniforms = {};
    uniforms.alphaCutoff = material.alphaCutoff;

    uniforms.baseColorFactor = glm::make_vec4(material.pbrData.baseColorFactor.data());
    if (material.pbrData.baseColorTexture.has_value()) {
        uniforms.flags |= MaterialUniformFlags::HasBaseColorTexture;
    }

    viewer->materials.emplace_back(uniforms);
    return true;
}

bool loadCamera(Viewer* viewer, fastgltf::Camera& camera) {
	// The following matrix math is for the projection matrices as defined by the glTF spec:
	// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#projection-matrices
	std::visit(fastgltf::visitor {
		[&](fastgltf::Camera::Perspective& perspective) {
			glm::mat4x4 mat(0.0f);

			assert(viewer->windowDimensions[0] != 0 && viewer->windowDimensions[1] != 0);
			auto aspectRatio = perspective.aspectRatio.value_or(
				static_cast<float>(viewer->windowDimensions[0]) / static_cast<float>(viewer->windowDimensions[1]));
			mat[0][0] = 1.f / (aspectRatio * tan(0.5f * perspective.yfov));
			mat[1][1] = 1.f / (tan(0.5f * perspective.yfov));
			mat[2][3] = -1;

			if (perspective.zfar.has_value()) {
				// Finite projection matrix
				mat[2][2] = (*perspective.zfar + perspective.znear) / (perspective.znear - *perspective.zfar);
				mat[3][2] = (2 * *perspective.zfar * perspective.znear) / (perspective.znear - *perspective.zfar);
			} else {
				// Infinite projection matrix
				mat[2][2] = -1;
				mat[3][2] = -2 * perspective.znear;
			}
			viewer->cameras.emplace_back(mat);
		},
		[&](fastgltf::Camera::Orthographic& orthographic) {
			glm::mat4x4 mat(1.0f);
			mat[0][0] = 1.f / orthographic.xmag;
			mat[1][1] = 1.f / orthographic.ymag;
			mat[2][2] = 2.f / (orthographic.znear - orthographic.zfar);
			mat[3][2] = (orthographic.zfar + orthographic.znear) / (orthographic.znear - orthographic.zfar);
			viewer->cameras.emplace_back(mat);
		},
	}, camera.camera);
	return true;
}

void drawMesh(Viewer* viewer, std::size_t meshIndex, glm::mat4 matrix) {
    auto& mesh = viewer->meshes[meshIndex];

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mesh.drawsBuffer);

    glUniformMatrix4fv(viewer->modelMatrixUniform, 1, GL_FALSE, &matrix[0][0]);

    for (auto i = 0U; i < mesh.primitives.size(); ++i) {
        auto& prim = mesh.primitives[i];
		auto& gltfPrimitive = viewer->asset.meshes[meshIndex].primitives[i];

		std::size_t materialIndex;
		auto& mappings = gltfPrimitive.mappings;
		if (!mappings.empty() && mappings[viewer->materialVariant].has_value()) {
			materialIndex = mappings[viewer->materialVariant].value() + 1; // Adjust for default material
		} else {
			materialIndex = prim.materialUniformsIndex;
		}

        auto& material = viewer->materialBuffers[materialIndex];
        glBindTextureUnit(0, prim.albedoTexture);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, material);
        glBindVertexArray(prim.vertexArray);

		// Update texture transform uniforms
		glUniform2f(viewer->uvOffsetUniform, 0, 0);
		glUniform2f(viewer->uvScaleUniform, 1.f, 1.f);
		glUniform1f(viewer->uvRotationUniform, 0);
		if (materialIndex != 0) {
			auto& gltfMaterial = viewer->asset.materials[materialIndex - 1];
			if (gltfMaterial.pbrData.baseColorTexture.has_value() && gltfMaterial.pbrData.baseColorTexture->transform) {
				auto& transform = gltfMaterial.pbrData.baseColorTexture->transform;
				glUniform2f(viewer->uvOffsetUniform, transform->uvOffset[0], transform->uvOffset[1]);
				glUniform2f(viewer->uvScaleUniform, transform->uvScale[0], transform->uvScale[1]);
				glUniform1f(viewer->uvRotationUniform, static_cast<float>(transform->rotation));
			}
		}

        glDrawElementsIndirect(prim.primitiveType, prim.indexType,
                               reinterpret_cast<const void*>(i * sizeof(Primitive)));
    }
}

void drawNode(Viewer* viewer, std::size_t nodeIndex, glm::mat4 matrix) {
    auto& node = viewer->asset.nodes[nodeIndex];
    matrix = getTransformMatrix(node, matrix);

    if (node.meshIndex.has_value()) {
        drawMesh(viewer, node.meshIndex.value(), matrix);
    }

    for (auto& child : node.children) {
        drawNode(viewer, child, matrix);
    }
}

void updateCameraNodes(Viewer* viewer, std::vector<fastgltf::Node*>& cameraNodes, std::size_t nodeIndex) {
	// This function recursively traverses the node hierarchy starting with the node at nodeIndex
	// to find any nodes holding cameras.
	auto& node = viewer->asset.nodes[nodeIndex];

	if (node.cameraIndex.has_value()) {
		if (node.name.empty()) {
			// Always have a non-empty string for the ImGui UI
			node.name = std::string("Camera ") + std::to_string(cameraNodes.size());
		}
		cameraNodes.emplace_back(&node);
	}

	for (auto& child : node.children) {
		updateCameraNodes(viewer, cameraNodes, child);
	}
}

void getCameraViewMatrix(Viewer* viewer, std::vector<fastgltf::Node*>& cameraNodes, std::size_t nodeIndex, glm::mat4 matrix) {
	auto& node = viewer->asset.nodes[nodeIndex];
	matrix = getTransformMatrix(node, matrix);

	if (node.cameraIndex.has_value() && &node == cameraNodes[*viewer->cameraIndex]) {
		viewer->viewMatrix = matrix;
	}

	for (auto& child : node.children) {
		getCameraViewMatrix(viewer, cameraNodes, child, matrix);
	}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No gltf file specified." << '\n';
        return -1;
    }
    auto gltfFile = std::string_view { argv[1] };
    Viewer viewer;

    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize glfw." << '\n';
        return -1;
    }

    auto* mainMonitor = glfwGetPrimaryMonitor();
    const auto* vidMode = glfwGetVideoMode(mainMonitor);

    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(static_cast<int>(static_cast<float>(vidMode->width) * 0.9f), static_cast<int>(static_cast<float>(vidMode->height) * 0.9f), "gl_viewer", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create window" << '\n';
        return -1;
    }
    glfwSetWindowUserPointer(window, &viewer);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// All of our callbacks need to be set before calling this so that it correctly chains them
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context." << '\n';
        return -1;
    }

    const auto *glRenderer = glGetString(GL_RENDERER);
    const auto *glVersion = glGetString(GL_VERSION);
    std::cout << "GL Renderer: " << glRenderer << "\nGL Version: " << glVersion << '\n';

    if (GLAD_GL_VERSION_4_6 != 1) {
        std::cerr << "Missing support for GL 4.6" << '\n';
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glMessageCallback, nullptr);

    // Compile the shaders
    GLuint program = GL_NONE;
    {
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

        const auto* frag = fragmentShaderSource.data();
        const auto* vert = vertexShaderSource.data();
        auto fragSize = static_cast<GLint>(fragmentShaderSource.size());
        auto vertSize = static_cast<GLint>(vertexShaderSource.size());

        glShaderSource(fragmentShader, 1, &frag, &fragSize);
        glShaderSource(vertexShader, 1, &vert, &vertSize);
        glCompileShader(fragmentShader);
        glCompileShader(vertexShader);
        if (!checkGlCompileErrors(fragmentShader))
            return -1;
        if (!checkGlCompileErrors(vertexShader))
            return -1;

        program = glCreateProgram();
        glAttachShader(program, fragmentShader);
        glAttachShader(program, vertexShader);
        glLinkProgram(program);
        if (!checkGlLinkErrors(program))
            return -1;

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
    }

	{
		// We just emulate the initial sizing of the window with a manual call.
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		windowSizeCallback(window, width, height);
	}

	// Load the glTF file
    auto start = std::chrono::high_resolution_clock::now();
    if (!loadGltf(&viewer, gltfFile)) {
        std::cerr << "Failed to parse glTF" << '\n';
        return -1;
    }

	// Add a default material
	auto& defaultMaterial = viewer.materials.emplace_back();
	defaultMaterial.baseColorFactor = glm::vec4(1.0f);
	defaultMaterial.alphaCutoff = 0.0f;
	defaultMaterial.flags = 0;

    // We load images first.
    auto& asset = viewer.asset;
    for (auto& image : asset.images) {
        loadImage(&viewer, image);
    }
    for (auto& material : asset.materials) {
        loadMaterial(&viewer, material);
    }
    for (auto& mesh : asset.meshes) {
        loadMesh(&viewer, mesh);
    }
	// Loading the cameras (possibly) requires knowing the viewport size, which we get using glfwGetWindowSize above.
	for (auto& camera : asset.cameras) {
		loadCamera(&viewer, camera);
	}
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "Loaded glTF file in " << diff.count() << "ms." << '\n';

    // Create the material uniform buffer
    viewer.materialBuffers.resize(viewer.materials.size(), GL_NONE);
    glCreateBuffers(static_cast<GLsizei>(viewer.materials.size()), viewer.materialBuffers.data());
    for (auto i = 0UL; i < viewer.materialBuffers.size(); ++i) {
        glNamedBufferStorage(viewer.materialBuffers[i], static_cast<GLsizeiptr>(sizeof(MaterialUniforms)),
                             &viewer.materials[i], GL_MAP_WRITE_BIT);
    }

    viewer.modelMatrixUniform = glGetUniformLocation(program, "modelMatrix");
    viewer.viewProjectionMatrixUniform = glGetUniformLocation(program, "viewProjectionMatrix");
	viewer.uvOffsetUniform = glGetUniformLocation(program, "uvOffset");
	viewer.uvScaleUniform = glGetUniformLocation(program, "uvScale");
	viewer.uvRotationUniform = glGetUniformLocation(program, "uvRotation");
    glUseProgram(program);

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

	auto& sceneIndex = viewer.sceneIndex = viewer.asset.defaultScene.value_or(0);

	// Give every scene a readable name, if not yet available
	for (std::size_t i = 0; i < asset.scenes.size(); ++i) {
		if (!asset.scenes[i].name.empty())
			continue;
		asset.scenes[i].name = std::string("Scene ") + std::to_string(i);
	}

	// We keep a list of all camera nodes present in the current scene.
	// When the cameraIndex has no value (because none is selected or there were none defined by the glTF),
	// a default free camera should be used.
	std::vector<fastgltf::Node*> cameraNodes;
	if (!viewer.asset.scenes.empty() && sceneIndex < viewer.asset.scenes.size()) {
		auto& scene = viewer.asset.scenes[sceneIndex];
		for (auto& node: scene.nodeIndices) {
			updateCameraNodes(&viewer, cameraNodes, node);
		}
	}

	// Set the initial direction (incl. pitch and yaw) and position of the camera.
	viewer.position = glm::vec3(2.f, 2.f, 2.f);
	viewer.direction = -viewer.position;
	{
		auto len = std::sqrt(std::pow(viewer.direction.x, 2) + std::pow(viewer.direction.z, 2));
		viewer.pitch = glm::degrees(std::tan(viewer.direction.y / len));
		viewer.yaw = -135.f;
	}

    viewer.lastFrame = static_cast<float>(glfwGetTime());
    while (glfwWindowShouldClose(window) != GLFW_TRUE) {
        auto currentFrame = static_cast<float>(glfwGetTime());
        viewer.deltaTime = currentFrame - viewer.lastFrame;
        viewer.lastFrame = currentFrame;

        // Reset the acceleration
        viewer.accelerationVector = glm::vec3(0.0f);

        // Updates the acceleration vector and direction vectors.
        glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Begin("gl_viewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
			auto& name = asset.scenes[sceneIndex].name;
			if (ImGui::BeginCombo("Scene", name.c_str(), ImGuiComboFlags_None)) {
				for (std::size_t i = 0; i < asset.scenes.size(); ++i) {
					const bool isSelected = i == sceneIndex;
					if (ImGui::Selectable(asset.scenes[i].name.c_str(), isSelected)) {
						sceneIndex = i;

						// Reset & update the camera nodes array
						cameraNodes.clear();
						auto& scene = viewer.asset.scenes[sceneIndex];
						for (auto& node : scene.nodeIndices) {
							updateCameraNodes(&viewer, cameraNodes, node);
						}
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Camera",
								  viewer.cameraIndex.has_value() ? cameraNodes[*viewer.cameraIndex]->name.c_str() : "Default",
								  ImGuiComboFlags_None)) {
				{
					// Default camera entry
					const bool isSelected = !viewer.cameraIndex.has_value();
					if (ImGui::Selectable("Default", isSelected)) {
						viewer.cameraIndex.reset();
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				for (std::size_t i = 0; i < cameraNodes.size(); ++i) {
					const bool isSelected = i == *viewer.cameraIndex;
					if (ImGui::Selectable(cameraNodes[i]->name.c_str(), isSelected)) {
						viewer.cameraIndex = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::BeginDisabled(asset.materialVariants.empty());
			const auto currentVariantName = asset.materialVariants.empty()
				? "N/A"
				: asset.materialVariants[viewer.materialVariant].c_str();
			if (ImGui::BeginCombo("Variant", currentVariantName, ImGuiComboFlags_None)) {
				for (std::size_t i = 0; i < asset.materialVariants.size(); ++i) {
					const bool isSelected = i == viewer.materialVariant;
					if (ImGui::Selectable(asset.materialVariants[i].c_str(), isSelected))
						viewer.materialVariant = i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::EndDisabled();
		}
		ImGui::End();

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (!asset.scenes.empty() && sceneIndex < asset.scenes.size()) {
			auto& scene = asset.scenes[sceneIndex];

			// Update the camera view and projection matrices
			if (viewer.cameraIndex.has_value()) {
				for (auto& sceneNode: scene.nodeIndices) {
					getCameraViewMatrix(&viewer, cameraNodes, sceneNode, glm::mat4(1.0f));
				}

				viewer.viewMatrix = glm::affineInverse(viewer.viewMatrix);
				viewer.projectionMatrix = viewer.cameras[viewer.cameraIndex.value()];
			} else {
				// Factor the deltaTime into the amount of acceleration
				viewer.velocity += (viewer.accelerationVector * 50.0f) * viewer.deltaTime;
				// Lerp the velocity to 0, adding deceleration.
				viewer.velocity = viewer.velocity + (2.0f * viewer.deltaTime) * (glm::vec3(0.0f) - viewer.velocity);
				// Add the velocity into the position
				viewer.position += viewer.velocity * viewer.deltaTime;
				viewer.viewMatrix = glm::lookAt(viewer.position, viewer.position + viewer.direction,
												glm::vec3(0.0f, 1.0f, 0.0f));

				auto aspectRatio = static_cast<float>(viewer.windowDimensions[0]) / static_cast<float>(viewer.windowDimensions[1]);
				viewer.projectionMatrix = glm::perspective(glm::radians(75.0f),
														   aspectRatio,
															0.01f, 1000.0f);
			}

			updateCameraMatrix(&viewer);

			for (auto& node: scene.nodeIndices) {
				drawNode(&viewer, node, glm::mat4(1.0f));
			}
		}

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    for (auto& mesh : viewer.meshes) {
        glDeleteBuffers(1, &mesh.drawsBuffer);

        for (auto& prim : mesh.primitives) {
            glDeleteVertexArrays(1, &prim.vertexArray);
			glDeleteBuffers(1, &prim.indexBuffer);
			glDeleteBuffers(1, &prim.vertexBuffer);
        }
    }

    glDeleteProgram(program);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
    glfwTerminate();
}
