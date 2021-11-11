#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

const int windowWidth = 1280;
const int windowHeight = 720;

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
  Ball(float _x, float _y, float _r = 20) {
    id = ++ballID;
    x = _x;
    y = _y;
    dx = 0;
    dy = 0;
    r = _r;
    mass = _r * _r;
    simTime = 0;
    lastX = x;
    lastY = y;
  }
};

int Ball::ballID = 0;

struct Collision {
  Ball* first;
  Ball* second;
};

struct Wall {};

class Game : public olc::PixelGameEngine {
 public:
  Game() { sAppName = "Marbles"; }

 private:
  const olc::vf2d gravity = {0, 100};

  olc::Sprite* spriteMarble;
  olc::Decal* decalMarble;

  olc::Sprite* spriteBackground;
  olc::Decal* decalBackground;

  float timer = 0.0;
  int frames = 0;
  int fps = 0;

  Ball* currentBall = nullptr;
  Wall* currentWall = nullptr;
  int currentOffsetX, currentOffsetY;

  bool simulationStopped = false;
  const int maxSimulationSteps = 8;

  struct Collision {
    Ball* first;
    Ball* second;
  };

  std::vector<Ball*> balls;
  std::vector<Collision*> collisions;

 public:
  bool OnUserCreate() override {
    spriteMarble = new olc::Sprite("./marble.png");
    decalMarble = new olc::Decal(spriteMarble);

    spriteBackground = new olc::Sprite("./background.png");
    decalBackground = new olc::Decal(spriteBackground);

    for (int i = 0; i < 100; i++) {
      balls.push_back(new Ball(float(rand() % ScreenWidth()),
                               float(rand() % ScreenHeight())));
    }

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

  bool inputs(float frameLength) {
    if (GetKey(olc::Key::ESCAPE).bPressed) {
      return false;
    }

    if (GetKey(olc::Key::SPACE).bPressed) {
      simulationStopped = true;
    }

    /*if (balls.length > 0) {
        if (keyIsDown(UP_ARROW)) {
            balls[0].dy = -100;
        }
        if (keyIsDown(LEFT_ARROW)) {
            balls[0].dx -= 300 * frameLength;
        } else if (keyIsDown(RIGHT_ARROW)) {
            balls[0].dx += 300 * frameLength;
        }
    }


    if (mouseIsPressed) {

        if (keyIsDown(CONTROL)) {

            // ADD A BALL IF CONTROL + MOUSE CLICK

            auto radius = 10;
            auto clear = true;
            for (auto ball : balls) {
                if (std::pow(mouseX - ball->x, 2) + std::pow(mouseY - ball->y,
    2) < std::pow(ball->r + radius, 2)) { clear = false; break;
                }
            }
            if (clear) addNewBall(mouseX, mouseY, radius);

        } else if (keyIsDown(SHIFT)) {

            if (currentWall !== null) {

                currentWall.end.x = mouseX;
                currentWall.end.y = mouseY;

            } else {

                currentWall = { start: { x: mouseX, y: mouseY }, end: { x:
    mouseX, y: mouseY }, r: 3 } walls.push(currentWall);
            }

        } else {

            if (currentBall === null) {

                for (auto ball : balls) {
                    if (std::pow(mouseX - ball->x, 2) + std::pow(mouseY -
    ball->y, 2) < std::pow(ball->r, 2)) { currentBall = ball; currentOffsetX =
    ball->x - mouseX; currentOffsetY = ball->y - mouseY; break;
                    }
                }
                for (auto wall : walls) {
                    if (std::pow(mouseX - wall.start.x, 2) + std::pow(mouseY -
    wall.start.y, 2) < std::pow(wall.r, 2)) { currentBall = wall.start;
                        currentOffsetX = wall.start.x - mouseX;
                        currentOffsetY = wall.start.y - mouseY;
                        break;
                    } else if (std::pow(mouseX - wall.end.x, 2) +
    std::pow(mouseY - wall.end.y, 2) < std::pow(wall.r, 2)) { currentBall =
    wall.end; currentOffsetX = wall.end.x - mouseX; currentOffsetY = wall.end.y
    - mouseY; break;
                    }
                }

            } else {

                const newX = mouseX + currentOffsetX;
                const newY = mouseY + currentOffsetY;

                currentball->x = newX;
                currentball->y = newY;

                if (currentball->hasOwnProperty("dx")) currentball->dx = 0;
                if (currentball->hasOwnProperty("dy")) currentball->dy = 0;

            }

        }

    } else {

        currentBall = null;
        currentWall = null;

    }*/

    return true;
  }

  void findWallCollisions(Ball* ball) {
    /*
      for (auto i = 0; i < walls.length; i++) {
        const wall = walls[i];

        const left = Math.min(wall.start.x, wall.end.x) - wall.r - ball->r;
        const top = Math.min(wall.start.y, wall.end.y) - wall.r - ball->r;
        const right = Math.max(wall.start.x, wall.end.x) + wall.r + ball->r;
        const bottom = Math.max(wall.start.y, wall.end.y) + wall.r + ball->r;

        const lastX = ball->lastX;  // posStack[ball->posStack.length - 1].x;
        const lastY = ball->lastY;  // posStack[ball->posStack.length - 1].y;

        const lastIn =
            lastX >= left && lastX <= right && lastY >= top && lastY <= bottom;
        const currentIn =
            ball->x >= left && ball->x <= right && ball->y >= top && ball->y <=
      bottom;

        if (!(lastIn || currentIn)) continue;

        const alphaX = wall.end.x - wall.start.x;
        const alphaY = wall.end.y - wall.start.y;

        const betaX = ball->x - wall.start.x;
        const betaY = ball->y - wall.start.y;

        const wallLength = std::pow(alphaX, 2) + std::pow(alphaY, 2);
        const dotProduct = alphaX * betaX + alphaY * betaY;
        const t = max(0, min(wallLength, dotProduct)) / wallLength;
        const u = wall.start.x + t * alphaX;
        const v = wall.start.y + t * alphaY;
        const distance =
            std::sqrt(std::pow(ball->x - u, 2) + std::pow(ball->y - v, 2));

        if (distance != = 0 && distance <= ball->r + wall.r) {
          const overlap = distance - ball->r - wall.r;
          ball->x -= overlap * (ball->x - u) / distance;
          ball->y -= overlap * (ball->y - v) / distance;

          collisions.push({
            first : ball,
            second : {
              id : -1,
              x : u,
              y : v,
              dx : -ball->dx,
              dy : -ball->dy,
              r : wall.r,
              mass : ball->mass
            }
          });
        }
      }*/
  }

  void resetWalls() {
    /*for (auto wall : walls) {
      wall.nx = -(wall.end.y - wall.start.y);
      wall.ny = (wall.end.x - wall.start.x) const d =
          std::sqrt(std::pow(wall.nx, 2) + std::pow(wall.ny, 2));
      wall.nx /= d;
      wall.ny /= d;
    }*/
  }

  void updateBall(Ball* ball) {
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

  void findBallCollisions(Ball* ball1) {
    /*for (auto i = 0; i < balls.length; i++) {
      const ball2 = balls[i];

      if (ball1.id == = ball2.id || ball2.frozen) continue;

      auto deltaX = ball1.x - ball2.x;
      auto deltaY = ball1.y - ball2.y;

      if (Math.abs(deltaX) > ball1.r + ball2.r ||
          Math.abs(deltaY) > ball1.r + ball2.r)
        continue;

      auto dSquared = std::pow(deltaX, 2) + std::pow(deltaY, 2);
      if (dSquared < std::pow(ball1.r + ball2.r, 2)) {
        collisions.push({first : ball1, second : ball2});

        const distance = std::sqrt(dSquared);

        auto overlap = 0.5 * (distance - ball1.r - ball2.r);

        if (ball2.frozen) {
          overlap *= 2;
        } else {
          ball2.x += overlap * deltaX / distance;
          ball2.y += overlap * deltaY / distance;
        }

        ball1.x -= overlap * deltaX / distance;
        ball1.y -= overlap * deltaY / distance;
      }
    }*/
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
    }
  }

  void postProcess(Ball* ball, float frameLength) {
    /*
    const lastX = ball->lastX;  // posStack[ball->posStack.length - 1].x;
    const lastY = ball->lastY;  // posStack[ball->posStack.length - 1].y;

    auto actualDistance =
        std::sqrt(std::pow(ball->x - lastX, 2) + std::pow(ball->y - lastY, 2));

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
    }*/
  }

  void testWallBreaches(Ball* ball) {
    /*auto x1, y1, x2, y2, x3, y3, x4, y4;
    auto ballD, ballUnitX, ballUnitY, rBallX, rBallY;

    auto reload = true;

    // for (auto wall : walls) {

    for (auto i = 0; i < walls.length; i++) {
      const wall = walls[i];

      if (reload) {
        x1 = ball->lastX;  // ball->posStack[ball->posStack.length - 1].x;
        y1 = ball->lastY;  // ball->posStack[ball->posStack.length - 1].y;
        x2 = ball->x;
        y2 = ball->y;

        ballD = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
        if (ballD == 0) return;

        reload = false;
      }

      auto x3 = wall.start.x;
      auto y3 = wall.start.y;
      auto x4 = wall.end.x;
      auto y4 = wall.end.y;

      const wallD = std::sqrt(std::pow(x4 - x3, 2) + std::pow(y4 - y3, 2));
      if (wallD == 0) continue;

      const d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
      if (d == 0) continue;

      const p = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / d;
      const q = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / d;

      if (p >= -ball->r / ballD && p <= 1 + ball->r / ballD &&
          q >= -wall.r / wallD && q <= 1 + wall.r / wallD) {
        const delta = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));

        auto nx = wall.nx;
        auto ny = wall.ny;

        auto u = x3 + q * (x4 - x3);
        auto v = y3 + q * (y4 - y3);

        auto lx = u - ball->lastX;  // ball->posStack[ball->posStack.length -
    1].x; auto ly = v - ball->lastY;  // ball->posStack[ball->posStack.length -
    1].y;

        ball->x = u - nx * (ball->r + wall.r);
        ball->y = v - ny * (ball->r + wall.r);
        ball->lastX = ball->x;
        ball->lastY = ball->y;
        // ball->posStack.push({ x: ball->x, y: ball->y, r: 255, g: 255, b: 0
    });

        reload = true;

        // ball->red = 0;
        // ball->green = 0;
        // ball->blue = 255;
      }
    }*/
  }

  void evaulateCollisions() {
    /*for (auto i = 0; i < collisions.length; i++) {
      const collision = collisions[i];

      const first = collision.first;
      const second = collision.second;

      const distance = std::sqrt(std::pow(first.x - second.x, 2) +
                                 std::pow(first.y - second.y, 2));

      if (distance == = 0) continue;

      const normalX = (second.x - first.x) / distance;
      const normalY = (second.y - first.y) / distance;

      const tangentX = -normalY;
      const tangentY = normalX;

      const firstDotProductTangent = first.dx * tangentX + first.dy * tangentY;
      const secondDotProductTangent = second.dx * tangentX + second.dy *
    tangentY;

      const firstDotProductNormal = first.dx * normalX + first.dy * normalY;
      const secondDotProductNormal = second.dx * normalX + second.dy * normalY;

      const firstMomentumConservation =
          (firstDotProductNormal * (first.mass - second.mass) +
           2 * second.mass * secondDotProductNormal) /
          (first.mass + second.mass);
      const secondMomentumConservation =
          (secondDotProductNormal * (second.mass - first.mass) +
           2 * first.mass * firstDotProductNormal) /
          (first.mass + second.mass);

      auto mu = 1;
      if (first.id == = -1 || second.id == = -1) mu = 0.5;

      first.dx = tangentX * firstDotProductTangent +
                 mu * normalX * firstMomentumConservation;
      first.dy = tangentY * firstDotProductTangent +
                 mu * normalY * firstMomentumConservation;
      if (second.id == = -1) {
        first.da = -firstDotProductTangent / first.r;
      } else {
        first.da = 0;
      }

      second.dx = tangentX * secondDotProductTangent +
                  mu * normalX * secondMomentumConservation;
      second.dy = tangentY * secondDotProductTangent +
                  mu * normalY * secondMomentumConservation;
      if (first.id == = -1) {
        second.da = secondDotProductTangent / second.r;
      } else {
        second.da = 0;
      }

    }*/
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
