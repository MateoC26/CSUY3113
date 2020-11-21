#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;

    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other)
{
    if (other == this) return false;

    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) return true;

    return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0)
            {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTopObj = true;
            }
            else if(velocity.y < 0)
            {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottomObj = true;
            }

            lastCollidedObject = object;
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX= fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0)
            {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRightObj = true;
            }
            else if (velocity.x < 0)
            {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeftObj = true;
            }

            lastCollidedObject = object;
        }
    }
}

void Entity::CheckCollisionsY(Map* map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTopMap = true;
    }
    else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTopMap = true;
    }
    else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTopMap = true;
    }

    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottomMap = true;
    }
    else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottomMap = true;
    }
    else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottomMap = true;
    }

    if (collidedBottomMap)
    {
        if (!map->IsSolid(bottom_left, &penetration_x, &penetration_y)) {
            holeLeft = true;
        }
        if (!map->IsSolid(bottom_right, &penetration_x, &penetration_y)) {
            holeRight = true;
        }
    }
}

void Entity::CheckCollisionsX(Map* map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeftMap = true;
    }

    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRightMap = true;
    }
}

void Entity::AIPatroller(Map* map)
{
    switch (aiState)
    {
    case IDLE:
        movement = glm::vec3(-1, 0, 0);
        animIndices = animRight;
        animFrames = 8;
        aiState = WALKING;
        break;

    case WALKING:
        if (holeLeft || collidedLeftMap || collidedLeftObj)
        {
            movement = glm::vec3(1, 0, 0);
            animIndices = animRight;
            animFrames = 8;
        }
        else if (holeRight || collidedRightMap || collidedRightObj)
        {
            movement = glm::vec3(-1, 0, 0);
            animIndices = animLeft;
            animFrames = 8;
        }
        break;

    case ATTACKING:
        break;
    }
}

void Entity::AIWaitAndGo(Entity* player)
{
    switch (aiState)
    {
    case IDLE:
        if (glm::distance(position, player->position) < 3.0f)
        {
            aiState = WALKING;
        }
        break;

    case WALKING:
        if (player->position.x < position.x)
        {
            movement = glm::vec3(-1, 0, 0);
        }
        else 
        {
            movement = glm::vec3(1, 0, 0);
        }
        break;

    case ATTACKING:
        break;
    }
}

void Entity::AI(Entity* player, Map* map)
{
    switch (aiType)
    {
    case PATROLLER:
        AIPatroller(map);
        break;

    case WAITANDGO:
        AIWaitAndGo(player);
        break;
    }
}

void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Map* map)
{
    if (isActive == false) return;

    collidedTopMap = false;
    collidedBottomMap = false;
    collidedLeftMap = false;
    collidedRightMap = false;

    collidedTopObj = false;
    collidedBottomObj = false;
    collidedLeftObj = false;
    collidedRightObj = false;

    holeLeft = false;
    holeRight = false;

    if (animIndices != NULL) {
        if (glm::length(movement) != 0) {
            animTime += deltaTime;

            if (animTime >= 0.10f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }
        }
        else {
            animIndices = animIdle;
            animFrames = 1;
            animIndex = 0;
        }
    }

    if (jump)
    {
        jump = false;
        justJumped = true;
        velocity.y += jumpPower;
    }

    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    position.y += velocity.y * deltaTime;  // Move on Y
    CheckCollisionsY(map);
    CheckCollisionsY(objects, objectCount);
    if (entityType == ENEMY) CheckCollisionsY(player, 1);

    position.x += velocity.x * deltaTime;  // Move on X
    CheckCollisionsX(map);
    CheckCollisionsX(objects, objectCount);
    if (entityType == ENEMY) CheckCollisionsX(player, 1);

    if (lastCollidedObject && lastCollidedObject->entityType == PLAYER)
    {
        if (collidedLeftObj)
        {
            player->collidedRightObj = true;
            lastCollidedObject = NULL;
        }
        else if (collidedRightObj)
        {
            player->collidedLeftObj = true;
            lastCollidedObject = NULL;
        }
    }

    if (player->lastCollidedObject && player->collidedBottomObj)
    {
        player->lastCollidedObject->lives--;
        player->lastCollidedObject = NULL;

        velocity.y += 4.0f;
        position.y += velocity.y * deltaTime;
    }

    if (player->collidedLeftObj || player->collidedRightObj)
    {
        player->lives -= 1;
        player->ResetPlayer();
    }

    if (entityType == ENEMY)
    {
        AI(player, map);
    }

    if (justJumped)
    {
        animIndices = animUp;
        animFrames = 1;
        animIndex = 0;

        if (collidedBottomMap)
        {
            justJumped = false;
            animIndices = animIdle;
            animFrames = 1;
            animIndex = 0;
        }
    }

    if (position.y < -8)
    {
        lives--;
        ResetPlayer();
    }

    if (lives <= 0)
    {
        isActive = false;
    }

    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) 
{
    if (isActive == false) return;

    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::ResetPlayer()
{
    position = glm::vec3(1, -5, 0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0, -9.81f, 0);
    velocity = glm::vec3(0);

    collidedTopObj = false;
    collidedBottomObj = false;
    collidedLeftObj = false;
    collidedRightObj = false;

    animIndices = animIdle;
    animFrames = 1;
    animIndex = 0;
}