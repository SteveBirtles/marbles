#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1280;
const int windowHeight = 720;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Raspberry Pi Club"; }

 private:
  olc::Sprite* spriteMarble;
  olc::Decal* decalMarble;

  olc::Sprite* spriteBackground;
  olc::Decal* decalBackground;

  float timer = 0.0;
  int frames = 0;
  int fps = 0;

  struct Ball {
    float x;
    float y;
    float r;
    float dx;
    float dy;
  };

  std::vector<Ball*> balls;

 public:
  bool OnUserCreate() override {
    spriteMarble = new olc::Sprite("./marble.png");
    decalMarble = new olc::Decal(spriteMarble);

    spriteBackground = new olc::Sprite("./background.png");
    decalBackground = new olc::Decal(spriteBackground);

    for (int i = 0; i < 100; i++) {
      balls.push_back(
          new Ball{float(rand() % ScreenWidth()),
                   float(rand() % ScreenHeight()), float(rand() % 20 + 20),
                   float(rand() % 1000 - 500), float(rand() % 1000 - 500)});
    }

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    if (!IsFocused()) {
      return true;
    }

    timer += fElapsedTime;
    frames++;
    if (timer > 1.0) {
      fps = frames;
      frames = 0;
      timer -= 1;
    }

    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    }

    for (auto& ball : balls) {
      ball->x += ball->dx * fElapsedTime;
      ball->y += ball->dy * fElapsedTime;

      if (ball->dx < 0 && ball->x < ball->r ||
          ball->dx > 0 && ball->x > ScreenWidth() - ball->r)
        ball->dx = -ball->dx;
      if (ball->dy < 0 && ball->y < ball->r ||
          ball->dy > 0 && ball->y > ScreenHeight() - ball->r)
        ball->dy = -ball->dy;
    }

    spriteBackground->SetPixel(
        olc::vi2d(rand() % spriteBackground->width,
                  rand() % spriteBackground->height),
        olc::Pixel(rand() % 256, rand() % 256, rand() % 256));
    
    decalBackground->Update();

    SetPixelMode(olc::Pixel::MASK);

    DrawDecal(olc::vf2d(ScreenWidth() / 2 - spriteBackground->width / 2,
                        ScreenHeight() / 2 - spriteBackground->height / 2),
              decalBackground, olc::vi2d(1, 1), olc::Pixel(128, 128, 128));

    for (auto& ball : balls) {
      DrawDecal(olc::vf2d(ball->x - ball->r, ball->y - ball->r), decalMarble,
                olc::vf2d(2 * ball->r / spriteMarble->width,
                          2 * ball->r / spriteMarble->height));
    }

    if (fps > 0) {
      DrawStringDecal(olc::vi2d(10, 10), "FPS " + std::to_string(fps));
    }

    return true;
  }

  bool OnUserDestroy() override {
    std::cout << "Closing application" << std::endl;
    return true;
  }
};

int main() {
  Game game;
  if (game.Construct(windowWidth, windowHeight, 1, 1)) game.Start();
  return 0;
}
