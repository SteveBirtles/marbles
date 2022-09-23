#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1280;
const int windowHeight = 720;
const int infinity = 1000000000;

struct Ball {
  static int ballID;
  int id;
  float x;
  float y;
  float dx;
  float dy;
  float r;
  float mass;
  float simTime;
  float lastX;
  float lastY;
  Ball(float _x, float _y, float _r, bool _temp = false, float _dx = 0,
       float _dy = 0, float _mass = -1) {
    if (_temp)
      id = -1;
    else
      id = ++ballID;
    x = _x;
    y = _y;
    dx = _dx;
    dy = _dy;
    r = _r;
    if (_mass == -1)
      mass = _r * _r;
    else
      mass = _mass;
    simTime = 0;
    lastX = x;
    lastY = y;
  }
};

int Ball::ballID = 0;

struct Collision {
  Ball *first;
  Ball *second;
};

struct Wall {
  static int wallID;
  int id;
  Ball *start;
  Ball *end;
  float r;
  float nx;
  float ny;
  Wall(Ball *_start, Ball *_end, float _r) {
    id = ++wallID;
    start = _start;
    end = _end;
    r = _r;
  }
};

int Wall::wallID = 0;

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Marbles"; }

 private:
  olc::vf2d gravity = {0, 100};

  olc::Sprite *spriteMarble;
  olc::Decal *decalMarble;

  olc::Sprite *spriteWall;
  olc::Decal *decalWall;

  olc::Sprite *spriteWallEnd;
  olc::Decal *decalWallEnd;

  olc::Sprite *spriteBackground;
  olc::Decal *decalBackground;

  float timer = 0.0;
  int frames = 0;
  int fps = 0;

  long wallCollisionCount = 0;
  long ballCollisionCount = 0;
  long wallBreachCount = 0;

  Ball *currentBall = nullptr;
  Wall *currentWall = nullptr;
  int currentOffsetX, currentOffsetY;

  bool simulationStopped = false;
  const int maxSimulationSteps = 8;

  struct Collision {
    Ball *first;
    Ball *second;
  };

  std::vector<Ball *> balls;
  std::vector<Collision *> collisions;
  std::vector<Wall *> walls;

 public:
  bool OnUserCreate() override {
    spriteMarble = new olc::Sprite("./marble.png");
    decalMarble = new olc::Decal(spriteMarble);

    spriteWall = new olc::Sprite("./wall.png");
    decalWall = new olc::Decal(spriteWall);

    spriteWallEnd = new olc::Sprite("./wallend.png");
    decalWallEnd = new olc::Decal(spriteWallEnd);

    spriteBackground = new olc::Sprite("./background.png");
    decalBackground = new olc::Decal(spriteBackground);

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override {
    timer += fElapsedTime;
    frames++;
    if (timer > 1.0) {
      fps = frames;
      frames = 0;
      timer -= 1;
    }

    if (!inputs(fElapsedTime)) return false;

    processDynamics(fElapsedTime);

    SetPixelMode(olc::Pixel::MASK);

    DrawDecal(olc::vf2d(ScreenWidth() / 2 - spriteBackground->width / 2,
                        ScreenHeight() / 2 - spriteBackground->height / 2),
              decalBackground, olc::vi2d(1, 1), olc::Pixel(64, 64, 64));

    for (auto ball : balls) {
      DrawDecal(olc::vf2d(ball->x - ball->r, ball->y - ball->r), decalMarble,
                olc::vf2d(2 * ball->r / spriteMarble->width,
                          2 * ball->r / spriteMarble->height));
    }

    for (auto wall : walls) {
      auto scale = olc::vf2d{float(2 * wall->r / spriteWallEnd->width),
                             float(2 * wall->r / spriteWallEnd->height)};
      DrawDecal(olc::vf2d{wall->start->x - wall->r, wall->start->y - wall->r},
                decalWallEnd, scale);
      DrawDecal(olc::vf2d{wall->end->x - wall->r, wall->end->y - wall->r},
                decalWallEnd, scale);
      DrawPolygonDecal(
          decalWall,
          std::vector<olc::vf2d>{{wall->start->x + wall->nx * wall->r,
                                  wall->start->y + wall->ny * wall->r},
                                 {wall->end->x + wall->nx * wall->r,
                                  wall->end->y + wall->ny * wall->r},
                                 {wall->end->x - wall->nx * wall->r,
                                  wall->end->y - wall->ny * wall->r},
                                 {wall->start->x - wall->nx * wall->r,
                                  wall->start->y - wall->ny * wall->r}},
          std::vector<olc::vf2d>{{0, 0}, {0, 1}, {1, 1}, {1, 0}});
    }

    if (fps > 0)
      DrawStringDecal(olc::vi2d(10, ScreenHeight() - 20),
                      "FPS " + std::to_string(fps));

    DrawStringDecal(olc::vi2d(10, 10), "Balls " + std::to_string(balls.size()));
    DrawStringDecal(olc::vi2d(10, 25), "Walls " + std::to_string(walls.size()));
    DrawStringDecal(olc::vi2d(10, 40), "Total ball collisions " +
                                           std::to_string(ballCollisionCount));
    DrawStringDecal(olc::vi2d(10, 55), "Total wall collisions " +
                                           std::to_string(wallCollisionCount));
    DrawStringDecal(olc::vi2d(10, 70),
                    "Total wall breaches " + std::to_string(wallBreachCount));

    DrawStringDecal(olc::vi2d(ScreenWidth() - 300, 10),
                    "Ctrl + Click = Add ball");
    DrawStringDecal(olc::vi2d(ScreenWidth() - 300, 25),
                    "Shift + Drag = Add wall");
    DrawStringDecal(olc::vi2d(ScreenWidth() - 300, 40),
                    "Drag  = Move ball or end of wall");
    DrawStringDecal(olc::vi2d(ScreenWidth() - 300, 55),
                    "Backspace  = Delete ball or wall");

    return true;
  }

  bool inputs(float frameLength) {
    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    }

    if (GetKey(olc::Key::SPACE).bPressed) {
      simulationStopped = true;
    }

    if (GetKey(olc::Key::BACK).bHeld) {
      if (currentBall != nullptr) {
        for (auto i = 0; i < walls.size();) {
          if (walls[i]->start->id == currentBall->id ||
              walls[i]->end->id == currentBall->id) {
            delete walls[i]->start;
            delete walls[i]->end;
            walls.erase(walls.begin() + i);
            currentBall = nullptr;
            break;
          } else {
            i++;
          }
        }
        if (currentBall != nullptr && currentBall->id > 0) {
          for (auto i = 0; i < balls.size();) {
            if (balls[i]->id == currentBall->id) {
              balls.erase(balls.begin() + i);
              break;
            } else {
              i++;
            }
          }
          delete currentBall;
          currentBall = nullptr;
        }
      }
    }

    if (GetMouse(0).bHeld) {
      if (GetKey(olc::Key::CTRL).bHeld) {
        auto radius = 10;
        auto clear = true;
        for (auto ball : balls) {
          if (std::pow(GetMouseX() - ball->x, 2) +
                  std::pow(GetMouseY() - ball->y, 2) <
              std::pow(ball->r + radius, 2)) {
            clear = false;
            break;
          }
        }
        if (clear) balls.push_back(new Ball(GetMouseX(), GetMouseY(), radius));

      } else if (GetKey(olc::Key::SHIFT).bHeld) {
        if (currentWall != nullptr) {
          currentWall->end->x = GetMouseX();
          currentWall->end->y = GetMouseY();

        } else {
          currentWall = new Wall(new Ball(GetMouseX(), GetMouseY(), 5),
                                 new Ball(GetMouseX(), GetMouseY(), 5), 5);
          walls.push_back(currentWall);
        }

      } else {
        if (currentBall != nullptr) {
          auto newX = GetMouseX() + currentOffsetX;
          auto newY = GetMouseY() + currentOffsetY;

          currentBall->x = newX;
          currentBall->y = newY;
          currentBall->dx = 0;
          currentBall->dy = 0;
        }
      }
    } else {
      currentBall = nullptr;
      currentWall = nullptr;

      for (auto ball : balls) {
        if (std::pow(GetMouseX() - ball->x, 2) +
                std::pow(GetMouseY() - ball->y, 2) <
            std::pow(ball->r, 2)) {
          currentBall = ball;
          currentOffsetX = ball->x - GetMouseX();
          currentOffsetY = ball->y - GetMouseY();
          break;
        }
      }

      if (currentBall == nullptr) {
        for (auto wall : walls) {
          if (std::pow(GetMouseX() - wall->start->x, 2) +
                  std::pow(GetMouseY() - wall->start->y, 2) <
              std::pow(wall->r, 2)) {
            currentBall = wall->start;
            currentOffsetX = wall->start->x - GetMouseX();
            currentOffsetY = wall->start->y - GetMouseY();
            break;
          } else if (std::pow(GetMouseX() - wall->end->x, 2) +
                         std::pow(GetMouseY() - wall->end->y, 2) <
                     std::pow(wall->r, 2)) {
            currentBall = wall->end;
            currentOffsetX = wall->end->x - GetMouseX();
            currentOffsetY = wall->end->y - GetMouseY();
            break;
          }
        }
      }

      if (currentBall == nullptr) {
        auto mouseBall = new Ball(GetMouseX(), GetMouseY(), 0, true);
        for (auto wall : walls) {
          auto n = nearestPoint(mouseBall, wall);

          if (n.x == infinity) continue;

          auto distance = std::sqrt(std::pow(mouseBall->x - n.x, 2) +
                                    std::pow(mouseBall->y - n.y, 2));

          if (distance != 0 && distance <= wall->r) {
            if (std::pow(mouseBall->x - wall->start->x, 2) +
                    std::pow(mouseBall->y - wall->start->y, 2) <
                std::pow(mouseBall->x - wall->end->x, 2) +
                    std::pow(mouseBall->y - wall->end->y, 2)) {
              currentBall = wall->start;
              currentOffsetX = wall->start->x - GetMouseX();
              currentOffsetY = wall->start->y - GetMouseY();
              break;
            } else {
              currentBall = wall->end;
              currentOffsetX = wall->end->x - GetMouseX();
              currentOffsetY = wall->end->y - GetMouseY();
              break;
            }
          }
        }
        delete mouseBall;
      }
    }
    return true;
  }

  void processDynamics(float frameLength) {
    for (auto ball : balls) {
      ball->simTime = frameLength;
    }

    resetWalls();

    for (auto j = 0; j < maxSimulationSteps; j++) {
      for (auto b : balls) {
        updateBall(b);

        if (simulationStopped) break;

        findBallCollisions(b);

        findWallCollisions(b);

        testWallBreaches(b);

        postProcess(b, frameLength);
      }

      evaulateCollisions();

      for (auto c : collisions) {
        if (c->first->id == -1) delete c->first;
        if (c->second->id == -1) delete c->second;
      }
      collisions.clear();
    }
  }

  void resetWalls() {
    for (auto wall : walls) {
      wall->nx = -(wall->end->y - wall->start->y);
      wall->ny = (wall->end->x - wall->start->x);
      auto d = std::sqrt(std::pow(wall->nx, 2) + std::pow(wall->ny, 2));
      wall->nx /= d;
      wall->ny /= d;
    }
  }

  void updateBall(Ball *ball) {
    if (ball->simTime > 0) {
      auto t = ball->simTime;
      auto v = std::sqrt(std::pow(ball->dx, 2) + std::pow(ball->dy, 2));
      if (t > ball->r / v) t = ball->r / v;

      ball->dx *= 1 - t;
      ball->dy *= 1 - t;

      ball->dx += gravity.x * t;
      ball->dy += gravity.y * t;

      ball->x += ball->dx * t;
      ball->y += ball->dy * t;
      ball->lastX = ball->x;
      ball->lastY = ball->y;
    }
  }

  void findBallCollisions(Ball *ball1) {
    for (auto ball2 : balls) {
      if (ball1->id == ball2->id) continue;

      auto deltaX = ball1->x - ball2->x;
      auto deltaY = ball1->y - ball2->y;

      if (std::abs(deltaX) > ball1->r + ball2->r ||
          std::abs(deltaY) > ball1->r + ball2->r)
        continue;

      auto dSquared = std::pow(deltaX, 2) + std::pow(deltaY, 2);
      if (dSquared < std::pow(ball1->r + ball2->r, 2)) {
        collisions.push_back(new Collision{ball1, ball2});
        ballCollisionCount++;

        auto distance = std::sqrt(dSquared);
        auto overlap = 0.5 * (distance - ball1->r - ball2->r);

        ball2->x += overlap * deltaX / distance;
        ball2->y += overlap * deltaY / distance;

        ball1->x -= overlap * deltaX / distance;
        ball1->y -= overlap * deltaY / distance;
      }
    }
  }

  olc::vf2d nearestPoint(Ball *ball, Wall *wall) {
    auto left = std::min(wall->start->x, wall->end->x) - wall->r - ball->r;
    auto top = std::min(wall->start->y, wall->end->y) - wall->r - ball->r;
    auto right = std::max(wall->start->x, wall->end->x) + wall->r + ball->r;
    auto bottom = std::max(wall->start->y, wall->end->y) + wall->r + ball->r;

    auto lastIn = ball->lastX >= left && ball->lastX <= right &&
                  ball->lastY >= top && ball->lastY <= bottom;
    auto currentIn = ball->x >= left && ball->x <= right && ball->y >= top &&
                     ball->y <= bottom;

    if (!(lastIn || currentIn)) return olc::vf2d{infinity, infinity};

    auto alphaX = wall->end->x - wall->start->x;
    auto alphaY = wall->end->y - wall->start->y;

    auto betaX = ball->x - wall->start->x;
    auto betaY = ball->y - wall->start->y;

    auto wallLength = float(std::pow(alphaX, 2) + std::pow(alphaY, 2));
    auto dotProduct = alphaX * betaX + alphaY * betaY;
    auto tau = std::max(0.0f, std::min(wallLength, dotProduct)) / wallLength;
    auto u = wall->start->x + tau * alphaX;
    auto v = wall->start->y + tau * alphaY;

    return olc::vf2d{u, v};
  }

  void findWallCollisions(Ball *ball) {
    for (auto wall : walls) {
      auto n = nearestPoint(ball, wall);

      if (n.x == infinity) continue;

      auto distance =
          std::sqrt(std::pow(ball->x - n.x, 2) + std::pow(ball->y - n.y, 2));

      if (distance != 0 && distance <= ball->r + wall->r) {
        auto overlap = distance - ball->r - wall->r;
        ball->x -= overlap * (ball->x - n.x) / distance;
        ball->y -= overlap * (ball->y - n.y) / distance;

        collisions.push_back(
            new Collision{ball, new Ball(n.x, n.y, wall->r, true, -ball->dx,
                                         -ball->dy, ball->mass)});
        wallCollisionCount++;
      }
    }
  }

  void testWallBreaches(Ball *ball) {
    float x1, y1, x2, y2, x3, y3, x4, y4;
    int ballD;
    float ballUnitX, ballUnitY, rBallX, rBallY;

    auto reload = true;

    for (auto wall : walls) {
      if (reload) {
        x1 = ball->lastX;
        y1 = ball->lastY;
        x2 = ball->x;
        y2 = ball->y;

        ballD = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
        if (ballD == 0) return;

        reload = false;
      }

      auto x3 = wall->start->x;
      auto y3 = wall->start->y;
      auto x4 = wall->end->x;
      auto y4 = wall->end->y;

      auto wallD = std::sqrt(std::pow(x4 - x3, 2) + std::pow(y4 - y3, 2));
      if (wallD == 0) continue;

      auto d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
      if (d == 0) continue;

      auto p = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / d;
      auto q = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / d;

      if (p >= -ball->r / ballD && p <= 1 + ball->r / ballD &&
          q >= -wall->r / wallD && q <= 1 + wall->r / wallD) {
        auto delta = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));

        auto nx = wall->nx;
        auto ny = wall->ny;

        auto u = x3 + q * (x4 - x3);
        auto v = y3 + q * (y4 - y3);

        auto lx = u - ball->lastX;

        auto ly = v - ball->lastY;

        ball->x = u - nx * (ball->r + wall->r);
        ball->y = v - ny * (ball->r + wall->r);
        ball->lastX = ball->x;
        ball->lastY = ball->y;

        wallBreachCount++;
      }

      reload = true;
    }
  }

  void postProcess(Ball *ball, float frameLength) {
    auto actualDistance = std::sqrt(std::pow(ball->x - ball->lastX, 2) +
                                    std::pow(ball->y - ball->lastY, 2));

    auto intendedSpeedSquared = std::pow(ball->dx, 2) + std::pow(ball->dy, 2);
    if (intendedSpeedSquared > 0) {
      auto intendedSpeed = std::sqrt(intendedSpeedSquared);
      auto actualTime = actualDistance / intendedSpeed;
      if (actualTime < frameLength) {
        ball->simTime -= actualTime;
      } else {
        ball->simTime -= frameLength;
      }
    } else {
      ball->simTime -= frameLength;
    }

    if (ball->x < -ball->r) {
      ball->x += ScreenWidth() + 2 * ball->r;
      ball->lastX = ball->x;
    }

    if (ball->y < -ball->r) {
      ball->y += ScreenHeight() + 2 * ball->r;
      ball->lastY = ball->y;
    }

    if (ball->x > ScreenWidth() + ball->r) {
      ball->x -= ScreenWidth() + 2 * ball->r;
      ball->lastX = ball->x;
    }

    if (ball->y > ScreenHeight() + ball->r) {
      ball->y -= ScreenHeight() + 2 * ball->r;
      ball->lastY = ball->y;
    }
  }

  void evaulateCollisions() {
    for (auto collision : collisions) {
      auto first = collision->first;
      auto second = collision->second;

      auto distance = std::sqrt(std::pow(first->x - second->x, 2) +
                                std::pow(first->y - second->y, 2));

      if (distance == 0) continue;

      auto normalX = (second->x - first->x) / distance;
      auto normalY = (second->y - first->y) / distance;

      auto tangentX = -normalY;
      auto tangentY = normalX;

      auto firstDotProductTangent = first->dx * tangentX + first->dy * tangentY;
      auto secondDotProductTangent =
          second->dx * tangentX + second->dy * tangentY;

      auto firstDotProductNormal = first->dx * normalX + first->dy * normalY;
      auto secondDotProductNormal = second->dx * normalX + second->dy * normalY;

      auto firstMomentumConservation =
          (firstDotProductNormal * (first->mass - second->mass) +
           2 * second->mass * secondDotProductNormal) /
          (first->mass + second->mass);
      auto secondMomentumConservation =
          (secondDotProductNormal * (second->mass - first->mass) +
           2 * first->mass * firstDotProductNormal) /
          (first->mass + second->mass);

      auto mu = 1.0f;
      if (first->id == -1 || second->id == -1) mu = 0.5;

      first->dx = tangentX * firstDotProductTangent +
                  mu * normalX * firstMomentumConservation;
      first->dy = tangentY * firstDotProductTangent +
                  mu * normalY * firstMomentumConservation;

      second->dx = tangentX * secondDotProductTangent +
                   mu * normalX * secondMomentumConservation;
      second->dy = tangentY * secondDotProductTangent +
                   mu * normalY * secondMomentumConservation;
    }
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
