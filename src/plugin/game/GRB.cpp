#include "GRB.h"


GRB::GRB()
    : game(std::make_unique<ProcessWrapper>("GRB.exe"))
{
    identityAddress = game->getBaseAddress() + 0x5A63514;
    avatarPositionAddress = game->getBaseAddress() + 0x5B57D50;
    cameraPositionAddress = game->getBaseAddress() + 0x5B57F00;
    cameraRotationAddress = game->getBaseAddress() + 0x5B57DC0;

    // 1: can move, 0: can not
    canMoveAddress = game->getBaseAddress() + 0x5B57CE1;
    // 1: normal, 2: drone
    movementModeAddress = game->getBaseAddress() + 0x5B57CE0;
    // foot: 11, car: 1, bike: 2, tank: 6, heli: 3, plane: 8
    vehicleTypeAddress = game->getBaseAddress() + 0x56721BC;
}

GRB::~GRB() {
    game.reset();
}

bool GRB::isConnected() {
    return game && game->isOpen();
}

GameState GRB::getState() {
    // update pointer chains
    avatarRotationAddress = game->resolvePointerChain({ 0x05B579B8, 0x90, 0x0, 0x28, 0xB0 });
    healthAddress = game->resolvePointerChain({ 0x05B5FE88, 0x0, 0x4C0, 0x998 });

    // fetch and preprocess data
    GameState state;

    // health
    game->readBuffer(healthAddress, (LPVOID)&(state.health), sizeof(float));

	// avatar position
    Vector3f avatarPosition = game->readVector(avatarPositionAddress).swapYandZ();
    state.mouthPosition = avatarPosition.toTS3();

    // avatar front
    Vector3f avatarRotation = game->readVector(avatarRotationAddress).swapYandZ();

    avatarRotation.y = 0.0f;
    avatarRotation = avatarRotation.normalize();
    state.mouthForward = avatarRotation.toTS3();

    // avatar top
    Vector3f avatarUp = Vector3f(0, 1, 0);
	state.mouthUp = avatarUp.toTS3();

	// camera position
    Vector3f cameraPosition = game->readVector(cameraPositionAddress).swapYandZ();
    cameraPosition.y = 0.0f;
    Vector3f resizedAvatarToCamera = (cameraPosition - avatarPosition).normalize(MAX_CAMERA_DISTANCE);

    cameraPosition = avatarPosition + resizedAvatarToCamera;
    state.earPosition = cameraPosition.toTS3();

    // camera front
    Vector3f cameraFront = game->readVector(cameraRotationAddress)
        .swapYandZ()
        .normalize();

    state.earForward = cameraFront.toTS3();

    // camera top
    Vector3f right = cameraFront.crossProduct(avatarUp);
    Vector3f cameraTop = right.crossProduct(cameraFront).normalize();

    state.earUp = cameraTop.toTS3();

    return state;
}
