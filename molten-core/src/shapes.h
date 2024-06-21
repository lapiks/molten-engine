namespace core::shapes {
  namespace cube {
    static float VERTICES[] = {
      // pos
      -0.5f, -0.5f,  0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 0.0f,     0.0f, 0.0f, -1.0f,// 0 
       0.5f, -0.5f,  0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 0.0f,     0.0f, 0.0f, -1.0f,// 1 
      -0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 1.0f,     0.0f, 0.0f, -1.0f,// 2 
       0.5f,  0.5f,  0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 1.0f,     0.0f, 0.0f, -1.0f,// 3 
      -0.5f, -0.5f, -0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 0.0f,     0.0f, 0.0f, -1.0f,// 4 
       0.5f, -0.5f, -0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 0.0f,     0.0f, 0.0f, -1.0f,// 5 
      -0.5f,  0.5f, -0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     1.0f, 1.0f,     0.0f, 0.0f, -1.0f,// 6 
       0.5f,  0.5f, -0.5f,   1.0f, 0.5f, 0.5f, 1.0f,     0.0f, 1.0f,     0.0f, 0.0f, -1.0f,// 7  
    };

    static uint16_t INDICES[] = {
      0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1
    };
  }

  namespace quad {
    float VERTICES[] = {
      // pos        // uvs
      -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
  }
}