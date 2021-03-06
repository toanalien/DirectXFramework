﻿#include "ObjectSpawner.h"
#include "PlayScene.h"
ObjectSpawner::ObjectSpawner(GVector2 position, int width, int height, eID type, int direction, float time, int number)
	: EmptyObject(eID::SPAWNER, position, width, height) {
	_createType = type;
	_time = time;
	_number = number;

	//_stopWatch = new StopWatch();

	_direction = direction;
	if(_direction == 0)
		_autoUpdateDirection = true;
	else _autoUpdateDirection = false;
	_isOnePerOne = false;

	_maxObject = 3;
	this->_bound.left = position.x;
	this->_bound.top = position.y;
	this->_bound.bottom = _bound.top - height;
	this->_bound.right = _bound.left + width;

}

ObjectSpawner::~ObjectSpawner() {}

void ObjectSpawner::init() {
}

void ObjectSpawner::update(float deltatime) {
	updateDirection();
	auto vpBounding = SceneManager::getInstance()->getCurrentScene()->getViewport()->getBounding();
	//this->_direction == 1 && this->getPositionX() > vpBounding.left&&this->getPositionY() - 70>vpBounding.bottom
	// check coi đi tới chưa, chưa tới mới tạo
	//
	
	auto _simon = ((PlayScene*)SceneManager::getInstance()->getCurrentScene())->getSimon();

	if (this->getPositionY() + 100 < _simon->getPositionY()) return;

	if (isRectangleIntersectedInDescartes(vpBounding, this->getBounding())
		&& !isContains(vpBounding, this->getBounding()) 
		&&( (this->getPositionY() + 20 > _simon->getPositionY()))
		) {
		//bool b = );
		if (_stopWatch == nullptr) {
			_time = 0;
			_stopWatch = new StopWatch();
		}
		else _time = 3000;
		if (_isOnePerOne == false) {
			if (_stopWatch->isStopWatch(_time)) {
				if (_number != -1 && _counter < _number) {
					_counter++;
					_listObjects.push_back(getObject(_createType));

					if (_counter < _number)
						_stopWatch->restart();
				}
				else if (_number == -1) {
					//if (_listObjects.size() < _maxObject)
					_listObjects.push_back(getObject(_createType));
					_stopWatch->restart();
				}
			}
		}
		else {
			if (_listObjects.size() == 0) {
				_listObjects.push_back(getObject(_createType));
			}
		}
	
	}

	//else if (this->getPositionX() <= vpBounding.left || this->getPositionY() <= vpBounding.bottom) {
	//	// Trong màn hình rồi thì huỷ luôn
	//	// qua luôn rồi thì hủy
	//	this->setStatus(eStatus::DESTROY);
	//	return;
	//}

	for (auto object : _listObjects) {
		object->update(deltatime);

		/*if (object->getScale().x > 0 && object->getPositionX() < vpBounding.left) {
			object->setStatus(eStatus::DESTROY);
		}
		else if (object->getScale().x < 0 && object->getPositionX() > vpBounding.right) {
			object->setStatus(eStatus::DESTROY);
		}
		else if (object->getPositionY() > vpBounding.top || object->getPositionY() < vpBounding.bottom) {
			object->setStatus(eStatus::DESTROY);
		}*/
	}

	this->deleteObject();
}

void ObjectSpawner::draw(LPD3DXSPRITE spriteHandle, Viewport * viewport) {
	for (auto object : _listObjects) {
		object->draw(spriteHandle, viewport);
	}
}

void ObjectSpawner::release() {
	SAFE_DELETE(_stopWatch);

	for (auto object : _listObjects) {
		SAFE_DELETE(object);
	}

	_listObjects.clear();
}

float ObjectSpawner::checkCollision(BaseObject * object, float dt) {
	//if (object->getId() != eID::LAND && object->getId() != eID::SIMON && object->getId() != eID::WHIP && object->getId() != eID::ITEM) {
	//	return 0.0f;
	//}

	for (auto item : _listObjects) {
		item->checkCollision(object, dt);
	}

	return 0.0f;
}

BaseObject * ObjectSpawner::getObject(eID id) {
	switch (id) {
	case MEDUSAHEAD:
	{
		auto medusaHead = new MedusaHead(eStatus::FLYING, _direction, this->getPosition(),
			MEDUSAHEAD_HORIZONTAL_VELOC,MEDUSAHEAD_AMPLITUDE,MEDUSAHEAD_FREQUENCY);
		medusaHead->init();
		auto pos = medusaHead->getPosition();
		return medusaHead;
		break;
	}
	case GHOST:
	{
		auto ghost = new Ghost(eStatus::HIDING,this->getPosition(),_direction);
		ghost->init();
		auto pos = ghost->getPosition();
		return ghost;
		break;
	}
	default:
		break;
	}
}

void  ObjectSpawner::updateDirection()
{
	if (!_autoUpdateDirection) return;
	auto _simon = ((PlayScene*)SceneManager::getInstance()->getCurrentScene())->getSimon();
	if (this->getPositionX() < _simon->getPositionX())
		_direction = 1;
	else if (this->getPositionX() > _simon->getPositionX())
		_direction = -1;
}

void ObjectSpawner::deleteObject() {
	for (auto object : _listObjects) {
		if (object->getStatus() == eStatus::DESTROY) {
			object->release();

			remove(_listObjects.begin(), _listObjects.end(), object);
			_listObjects.pop_back();

			delete object;

			break;
		}
	}
}

RECT ObjectSpawner::getBounding() {
	return this->_bound;
}

vector<BaseObject*> ObjectSpawner::getObjects() {
	return _listObjects;
}

void ObjectSpawner::setOnePerOne(bool enable) {
	_isOnePerOne = enable;
}

bool ObjectSpawner::isOnePerOne() {
	return _isOnePerOne;
}

void ObjectSpawner::setMaxNumber(int number) {
	_maxObject = number;
}

int ObjectSpawner::getMaxNumber() {
	return _maxObject;
}




