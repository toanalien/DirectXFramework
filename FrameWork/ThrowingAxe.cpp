﻿#include "ThrowingAxe.h"
#include "BaseEnemy.h"

ThrowingAxe::ThrowingAxe(GVector2 startPosition, eItemType type, eDirection dir) : Weapon(startPosition, type, dir, eItemID::AXE)
{
	_damage = 2;
}


ThrowingAxe::~ThrowingAxe()
{
}

void ThrowingAxe::init()
{
	_sprite = SpriteManager::getInstance()->getSprite(eID::ITEM);
	_sprite->setFrameRect(SpriteManager::getInstance()->getSourceRect(eID::ITEM, "axe"));

	if (_type == eItemType::PICKED_UP)
		HeartCounter::plusHeart(-1);

	Weapon::initCommonComponent();

	initWeaponComponent();
}


void ThrowingAxe::update(float deltatime)
{
	Weapon::update(deltatime);
	if (abs(this->getPositionX() - this->_startPosition.x) > THROWING_AXE_DISTANCE.x)
		this->setStatus(eStatus::DESTROY);
}

void ThrowingAxe::draw(LPD3DXSPRITE spriteHandler, Viewport* viewport)
{
	Weapon::draw(spriteHandler, viewport);
}

void ThrowingAxe::release()
{

}

void ThrowingAxe::initWeaponComponent()
{
	if (_type != eItemType::PICKED_UP) return;

	GVector2 veloc = this->initVeloc(THROWING_AXE_DISTANCE);

	auto move = (Movement*)this->_componentList["Movement"];
	move->setVelocity(veloc);

	auto gravity = (Gravity*)this->_componentList["Gravity"];
	gravity->setStatus(eGravityStatus::FALLING_DOWN);
	gravity->setGravity(GVector2(0, -800));

	RotateMovement* rotateMovement = new RotateMovement(_sprite);
	_componentList["RotateMovement"] = rotateMovement;
}
