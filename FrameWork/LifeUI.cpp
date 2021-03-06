#include "LifeUI.h"


LifeUI::LifeUI(GVector2 position,string text,string spritePath, int number,int HP) :EmptyObject(eID::LIFE_ICON,position,0,0)
{
	_hp = HP;
	_life = number;
	_spritePath = spritePath;

	_text = new Text(L"Arial", text, position.x, position.y, 21);

	position += GVector2(TEXT_TAB, 0);

	for (int i = 0; i < MAX_HP_NUMBER; i++)
	{
		auto sprite = SpriteManager::getInstance()->getSprite(eID::ITEM);
		sprite->setFrameRect(SpriteManager::getInstance()->getSourceRect(eID::ITEM, spritePath));
		sprite->setScale(SCALE_FACTOR);
		_listIcons.push_back(sprite);
		_listIcons.back()->setPosition(position.x + (_listIcons.back()->getFrameWidth() + GAP) * i, position.y + _text->getFontHeight()/2);
	}

}


LifeUI::~LifeUI()
{
}


void LifeUI::init()
{

}

void LifeUI::update(float deltatime)
{
	if (!_isGainingHitpoint) return;
	if (_gainHitPoint > 0 && _delay > 200)
	{
		_gainHitPoint -= 1;
		int hp = _hp + 1;
		this->setHPNumber(hp);
		_delay = 0;
	}
	else if (_gainHitPoint > 0 && _delay < 200)
	{
		_delay += deltatime;
	}
	else if (_gainHitPoint == 0)
	{
		_isGainingHitpoint = false;
	}

}


void LifeUI::gainHitpoint(int hitPoint)
{
	_gainHitPoint = hitPoint;
	_isGainingHitpoint = true;
}


void LifeUI::draw(LPD3DXSPRITE spriteHandler, Viewport* viewport)
{
	_text->draw();
	for (auto icon : _listIcons)
		icon->render(spriteHandler);
}

void LifeUI::release()
{
	while (!_listIcons.empty())
	{
		SAFE_DELETE(_listIcons.back());
		_listIcons.pop_back();
	}
}

void LifeUI::setHPNumber(int number)
{
	if (_hp == number || number > MAX_HP_NUMBER)
		return;

	if (number < 0 || _life <= 0)
		number = 0;

	_hp = number;
	
	for (int i = 0; i < _hp; i++)
	{
		_listIcons[i]->setFrameRect(SpriteManager::getInstance()->getSourceRect(eID::ITEM, _spritePath));
	}

	for (int i = _hp; i < MAX_HP_NUMBER;i++)
	{
		_listIcons[i]->setFrameRect(SpriteManager::getInstance()->getSourceRect(eID::ITEM, "white_life_icon"));
	}
}

int LifeUI::getHPNumber()
{
	return _hp;
}

void LifeUI::setLifeNumber(int number)
{
	_life = number;
}

int LifeUI::getLifeNumber()
{
	return _life;
}

void  LifeUI::dropHitPoint(int _drop)
{
	int hp = _hp - _drop;
	if (hp <= 0)
		hp = 0;
	this->setHPNumber(hp);
}