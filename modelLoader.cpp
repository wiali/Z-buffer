#include "modelLoader.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures){
    this->vertices = vertices;
    this->indices =  indices;
    this->textures = textures;
}

ModelLoader::ModelLoader(std::string path){
    loadModel(path);
}

void ModelLoader::loadModel(std::string path){
    Assimp::Importer importer; 
    const aiScene* scene = importer.ReadFile( path.c_str(),  
      aiProcess_Triangulate            | 
      aiProcess_FlipUVs); 

    if( !scene ){
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void ModelLoader::processNode(aiNode *node, const aiScene *scene){
    for(unsigned int i = 0; i < node->mNumMeshes; ++i){
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));;
    }

    for(unsigned int i = 0; i < node->mNumChildren; ++i){
        processNode(node->mChildren[i], scene);
    }
}

Mesh ModelLoader::processMesh(aiMesh *mesh, const aiScene *scene){
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; ++i){
        Vertex vertex;
        // vertex postion
        glm::vec3 vect;
        vect.x = mesh->mVertices[i].x;
        vect.y = mesh->mVertices[i].y;
        vect.z = mesh->mVertices[i].z;
        vertex.Position = vect;

        // normal
        vect.x = mesh->mNormals[i].x;
        vect.y = mesh->mNormals[i].y;
        vect.z = mesh->mNormals[i].z;
        vertex.Normal = vect;

        // coordinates
        if(mesh->mTextureCoords[0]){
            glm::vec2 vect;
            vect.x = mesh->mTextureCoords[0][i].x; 
            vect.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vect;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);  
        vertices.push_back(vertex);       
    }

    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; ++i){
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // process material

    return Mesh(vertices, indices, textures);  
}