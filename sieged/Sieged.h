#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>

namespace blib { class Texture; class Animation; class FBO; class Shader; class AnimatableSprite;  }



class Sieged : public blib::App
{
	blib::Texture* tiles;



	blib::MouseState prevMouseState;

public:
	Sieged();
	virtual void init() override;
	virtual void update(double elapsedTime) override;
	virtual void draw() override;
};