﻿#pragma once
#include"define.h"
#include"Viewport.h"
#include"Director.h"
/*
	Abstract Class cho một màn chơi của game.
	Bao gồm các hàm giống như trong game.h
*/
class Scene
{
public:
	Scene();
	~Scene();

	bool virtual init() = 0;
	void virtual updateInput(float dt) = 0;
	void virtual update(float dt) = 0;
	void virtual draw(LPD3DXSPRITE spriteHandle) = 0;
	void virtual release() = 0;

	Viewport* getViewport();
	Director* getDirector();
	eID getIdLevel();
protected:
	Viewport* _viewport;
	Director* _director;
	eID _idLevel;
};

