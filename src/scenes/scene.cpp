#include "scene.hpp"

using namespace std;

Scene::Scene()
    : m_is_set_up(false)
{
}

Scene::~Scene()
{
}

void Scene::setup()
{
    m_is_set_up = true;
}
