#include "pch.h"

typedef sf::Event sfe;
typedef sf::Keyboard sfk;

struct Spherical
{
    float distance, theta, phi;
    Spherical(float gdistance, float gtheta, float gphi) : distance(gdistance), theta(gtheta), phi(gphi) { }
    float getX() { return 1.0 * cos(phi); }
    float getY() { return 1.0 * sin(theta); }
    float getZ() { return 1.0 * sin(phi); }
};

float sphereForce = 0.0f;
float sphereAngle = 0.0f;
float t = 0;
bool wasClisked = false;

float ballPosY = 3.0f;
float ballPosZ = 0.0f;
float ballPosX = 0.0f;

Spherical camera(1.0f, 0.0f, 0.0f);
sf::Vector3f pos(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rot(0.0f, 0.0f, 0.0f);
bool perspective_projection = true;
float fov = 80.0f;

float* pos_offsets[3] = { &pos.x, &pos.y, &pos.z };
float* scale_offsets[3] = { &scale.x, &scale.y, &scale.z };
float* rot_offsets[3] = { &rot.x, &rot.y, &rot.z };

sf::Image image;
sf::Image image2;

GLuint textureID;
GLuint textureID2;

sf::Texture TEXid;


const double maxFrameRate = 60.0;
const double frameTime = 1.0 / maxFrameRate;


float distance2 = 0.0;


void limitFrameRate(std::chrono::time_point<std::chrono::high_resolution_clock>& startTime) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();

    if (elapsedTime < frameTime) {
        double sleepTime = frameTime - elapsedTime;

        
        std::chrono::microseconds sleepDuration(static_cast<long long>(sleepTime * 1e6));
        std::this_thread::sleep_for(sleepDuration);
    }

    startTime = std::chrono::high_resolution_clock::now();
}

void initOpenGL(void)
{
    glClearColor(0.0, 0.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    if (!image.loadFromFile("stone.jpg")) {
        printf("Error loading texture image\n");
        return;
    }



    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    TEXid.loadFromFile("grass.jpg");
    TEXid.generateMipmap();
    
    glEnable(GL_TEXTURE_2D);


}

void reshapeScreen(sf::Vector2u size)
{
    glViewport(0, 0, (GLsizei)size.x, (GLsizei)size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective_projection) gluPerspective(fov, (GLdouble)size.x / (GLdouble)size.y, 0.1, 100.0);
    else glOrtho(-1.245 * ((GLdouble)size.x / (GLdouble)size.y), 1.245 * ((GLdouble)size.x / (GLdouble)size.y), -1.245, 1.245, -3.0, 12.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawScene()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Spherical north_of_camera(camera.distance, camera.theta, camera.phi + 0.01f);
    gluLookAt(-camera.getX(), 3.4, -camera.getZ(),
       40.0 * camera.getX(), 40*camera.getY(), 40.0 * camera.getZ(),
        0.0, 0.1, 0);



    //transformacje
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(rot.x, 1, 0, 0);
    glRotatef(rot.y, 0, 1, 0);
    glRotatef(rot.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLUquadricObj* qobj = gluNewQuadric();
    
    gluQuadricTexture(qobj, GL_TRUE);

    glPushMatrix();
    glColor3f(0.78f, 0.78f, 0.78f);
    glTranslatef(ballPosX, ballPosY, ballPosZ);
    gluSphere(qobj, 0.2, 15, 10);
    glPopMatrix();



    glColor3f(0.1, 0.9, 0.1);  // Set color to green




    //glBindTexture(GL_TEXTURE_2D, textureID2);
    glEnable(GL_TEXTURE_2D);
    sf::Texture::bind(&TEXid);
    glBegin(GL_QUADS);
    for (int i = -100; i < 100; i=i+2) {
        for (float j = -100; j < 101; j = j + 2) {
      
            glTexCoord2f(0.0, 0.0); glVertex3f(i, 0.0, j);
            glTexCoord2f(1.0, 0.0); glVertex3f(i + 10, 0.0, j);
            glTexCoord2f(1.0, 1.0); glVertex3f(i + 10, 0.0, j + 10);
            glTexCoord2f(0.0, 1.0); glVertex3f(i, 0.0, j + 10);
            
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0, 0, 0);

    const int numSegments = 360;
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);  
    for (int i = 0; i <= numSegments; ++i) {
        float theta2 = (static_cast<float>(i) / static_cast<float>(numSegments)) * 2.0f * 3.1415;
        float x = 5 * std::cos(theta2);
        float y = 5 * std::sin(theta2);
        glVertex3f(x, 0.1, y);  
    }
    glEnd();


    glColor3f(0, 0, 1.0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);  
    for (int i = 0; i <= numSegments; ++i) {
        float theta2 = (static_cast<float>(i) / static_cast<float>(numSegments)) * 2.0f * 3.1415;
        float x = 10 * std::cos(theta2);
        float y = 10 * std::sin(theta2);
        glVertex3f(x, 0.1, y);  
    }
    glEnd();


    glColor3f(1.0, 0, 1.0);
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);  
    for (int i = 0; i <= numSegments; ++i) {
        float theta2 = (static_cast<float>(i) / static_cast<float>(numSegments)) * 2.0f * 3.1415;
        float x = 20 * std::cos(theta2);
        float y = 20 * std::sin(theta2);
        glVertex3f(x, 0.1, y);  
    }
    glEnd();


    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(0.0f, 0.0f, 0.0f);  
    for (int i = 0; i <= numSegments; ++i) {
        float theta2 = (static_cast<float>(i) / static_cast<float>(numSegments)) * 2.0f * 3.1415;
        float x = 50 * std::cos(theta2);
        float y = 50 * std::sin(theta2);
        glVertex3f(x, 0.1, y);  
    }
    glEnd();
    
    
}



int main()
{
    bool running = true;
    sf::ContextSettings context(24, 0, 0, 4, 5);
    sf::RenderWindow window(sf::VideoMode(1280, 1024), "Open GL Lab1 04", 7U, context);
    sf::Clock deltaClock;

    ImGui::SFML::Init(window);

    window.setVerticalSyncEnabled(true);

    reshapeScreen(window.getSize());
    initOpenGL();

    while (running)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        sfe event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sfe::Closed || (event.type == sfe::KeyPressed && event.key.code == sfk::Escape)) running = false;
            if (event.type == sfe::Resized) reshapeScreen(window.getSize());
        }

        if (sfk::isKeyPressed(sfk::Down)) { camera.theta -= 0.0174533f; if (camera.theta < -90.0f) camera.theta = -90.0f; }
        if (sfk::isKeyPressed(sfk::Up)) { camera.theta += 0.0174533f; if (camera.theta > (float)(2.0 * std::numbers::pi)) camera.theta = (float)(2.0 * std::numbers::pi); }
        if (sfk::isKeyPressed(sfk::Right)) { camera.phi += 0.0174533f;  if (camera.phi > (float)(2.0*std::numbers::pi)) camera.phi = (float)(2.0*std::numbers::pi); }
        if (sfk::isKeyPressed(sfk::Left)) { camera.phi -= 0.0174533f; if (camera.phi < -360.0f) camera.phi = -360.0f; }
        if (sfk::isKeyPressed(sfk::Space)) { wasClisked = true; }
        if (sfk::isKeyPressed(sfk::R)) { ballPosX = 0;ballPosY = 3;ballPosZ = 0;wasClisked = false;t = 0;}
        
        if (ballPosY + (sphereForce * sin(sphereAngle) * (t+0.2) + 0.5 * (-0.2) * (t+0.2) * (t+0.2))/5 >= 0.1 && wasClisked == true) {
            t = t + 0.2;
            ballPosY = ballPosY+(sphereForce*sin(sphereAngle)*t+0.5*( - 0.2)*t*t)/5;
            ballPosX += cos(sphereAngle) * sphereForce*camera.getX()/5;
            ballPosZ += cos(sphereAngle) * sphereForce* camera.getZ()/5;
            distance2 = sqrt(ballPosX * ballPosX + ballPosZ * ballPosZ);

        }
        else {
            t = 0;
            wasClisked = false;
            
        }

      
        drawScene();

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Camera");
        ImGui::SliderAngle("gora/dol", &camera.theta, -90.0f, 90.0f);
        ImGui::SliderAngle("lewo/prawo", &camera.phi, -360.0f, 360.0f);
        if (ImGui::Checkbox("Perspective projection", &perspective_projection)) reshapeScreen(window.getSize());
        if (ImGui::SliderFloat("FoV", &fov, 10.0f, 90.0f)) reshapeScreen(window.getSize());
        ImGui::End();

        ImGui::Begin("Controls");
        ImGui::SliderFloat("Force", &sphereForce, 0.0f, 3.0f);
        ImGui::SliderAngle("Angle", &sphereAngle, 0.0f, 90-.0f);
        if (ImGui::Button("Animate Sphere")) {
            wasClisked = true;
            
        }
        if (ImGui::Button("Reset")) {

            ballPosX = 0;
            ballPosY = 3;
            ballPosZ = 0;
            wasClisked = false;
            t = 0;

        }
        ImGui::Text("Distance: %.2f", distance2);
        ImGui::Text("Red: 5m");
        ImGui::Text("Blue: 10m");
        ImGui::Text("Pink: 20m");
        ImGui::Text("White: 50m");
        ImGui::End();
        limitFrameRate(startTime);
        ImGui::SFML::Render(window);

        window.display();
        
    }
    ImGui::SFML::Shutdown();
    return 0;
}
