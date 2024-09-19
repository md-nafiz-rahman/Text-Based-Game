#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class Room {
    public: string id;
    string desc;
    string altDesc;
    string cctvDesc;
    string validDesc;
    bool useAltDesc = false;
    bool isTeleportRoom = false;
    map < string,
    string > exits;
    string status;
    string keyRequired;

    Room() {}

    Room(const json & j) {
        id = j["id"];
        desc = j["desc"];
        exits = j["exits"].get < map < string, string >> ();

        // Optional JSON fields are checked and assigned if they exist in the JSON data.	  
        if (j.find("alt_desc") != j.end()) {
            altDesc = j["alt_desc"];
        }
        if (j.find("valid_desc") != j.end()) {
            validDesc = j["valid_desc"];
        }
        if (j.find("cctv_desc") != j.end()) {
            cctvDesc = j["cctv_desc"];
        }
        if (j.find("status") != j.end()) {
            status = j["status"];
        } else {
            status = "unlocked";
        }
        if (j.find("key_required") != j.end()) {
            keyRequired = j["key_required"];
        }
        if (j.find("isTeleportRoom") != j.end()) {
            isTeleportRoom = j["isTeleportRoom"];
        }
    }
};

class Object {
    public: string id;
    string desc;
    string initialRoom;
    int requiredCode;
    map < string,
    map < string,
    json >> onSuccessActions;
    int healthScore = 0;
    bool isAvailable = true;
    bool isShield = false;
    bool isUsable = false;
    bool isOpenable = false;
    bool isBreakable = false;
    bool discardAfterUse = false;
    string requiresKey = "";
    vector < string > contents;
    vector < string > breakBy;
    vector < string > displayRooms;

    Object() {}

    Object(const json & j) {
        id = j["id"];
        desc = j["desc"];
        initialRoom = j["initialroom"];

        // Check for optional fields in JSON data and initialize them if present.
        if (j.find("healthscore") != j.end()) {
            healthScore = j["healthscore"];
        }
        if (j.find("isShield") != j.end()) {
            isShield = j["isShield"];
        }
        if (j.find("isAvailable") != j.end()) {
            isAvailable = j["isAvailable"];
        }
        if (j.find("isUsable") != j.end()) {
            isUsable = j["isUsable"];
        }
        if (j.find("isOpenable") != j.end()) {
            isOpenable = j["isOpenable"];
        }
        if (j.find("isBreakable") != j.end()) {
            isBreakable = j["isBreakable"];
        }
        if (j.find("requiresKey") != j.end()) {
            requiresKey = j["requiresKey"];
        }
        if (j.find("contents") != j.end()) {
            contents = j["contents"].get < vector < string >> ();
        }
        if (j.find("breakBy") != j.end()) {
            breakBy = j["breakBy"].get < vector < string >> ();
        }
        if (j.find("required_code") != j.end()) {
            requiredCode = j["required_code"];
        }
        if (j.find("on_success") != j.end()) {
            onSuccessActions = j["on_success"]["update_room"].get < map < string, map < string, json >>> ();
        }
        if (j.find("display_rooms") != j.end()) {
            displayRooms = j["display_rooms"].get < vector < string >> ();
        }
        if (j.find("discardAfterUse") != j.end()) {
            discardAfterUse = j["discardAfterUse"];
        }
    }
};

class Enemy {
    public: string id;
    string desc;
    int aggressiveness;
    vector < string > killedBy;
    string initialRoom;
    string introMsg;
    string successfulKillMsg;
    string unsuccessfulKillMsg;
    string successfulEscapeMsg;
    string unsuccessfulEscapeMsg;
    string requiredAnswer;
    map < string,
    string > onDefeatUpdateRoom;
    map < string,
    bool > onDefeatUpdateObject;

    Enemy() {}

    Enemy(const json & j) {
        id = j["id"];
        desc = j["desc"];
        aggressiveness = j["aggressiveness"];
        killedBy = j["killedby"].get < vector < string >> ();
        initialRoom = j["initialroom"];
        introMsg = j.value("intro_msg", "");
        successfulKillMsg = j.value("successful_kill_msg", "");
        unsuccessfulKillMsg = j.value("unsuccessful_kill_msg", "");
        successfulEscapeMsg = j.value("successful_escape_msg", "");
        unsuccessfulEscapeMsg = j.value("unsuccessful_escape_msg", "");
        requiredAnswer = j.value("requiredAnswer", "");

        // Initializing room and object updates on enemy defeat, if specified in JSON data.
        if (j.find("on_defeat_update_room") != j.end()) {
            onDefeatUpdateRoom = j["on_defeat_update_room"].get < map < string, string >> ();
        }

        if (j.find("on_defeat_update_object") != j.end()) {
            onDefeatUpdateObject = j["on_defeat_update_object"].get < map < string, bool >> ();
        }
    }
};

class Player {
    public: string currentRoom;
    vector < string > inventory;
    int health = 100;
    bool justMoved =
    false;

    Player(const string & startRoom): currentRoom(startRoom) {}
    // Method to adjust the player's health.
    void adjustHealth(int amount) {
        health += amount;
        if (health > 100) health = 100;
        if (health < 0) health = 0;
    }
};

class Game {
    public: map < string,
    Room > rooms;
    map < string,
    Object > objects;
    map < string,
    Enemy > enemies;
    Player player;
    json gameObjective;
    bool unsuccessfulKillAttempt = false;
    bool statueBroken = false;

    Game(const json & j): player(j["player"]["initialroom"]),
    gameObjective(j["objective"]) {
        // Initializing rooms, objects and enemies
        for (const auto & room: j["rooms"]) {
            rooms[room["id"]] = Room(room);
        }

        for (const auto & object: j["objects"]) {
            objects[object["id"]] = Object(object);
        }

        for (const auto & enemy: j["enemies"]) {
            enemies[enemy["id"]] = Enemy(enemy);
        }
    }

    void startGame() {
        cout << "Welcome to the Adventure Game!" << endl;
        displayRoomDetails();
        while (true) {
            cout << endl;
            cout << "> ";
            string command;
            getline(cin, command);
            processCommand(command);

            if (checkWinCondition()) {
                cout << "Congratulations! You have achieved the objective!" <<
                    endl;
                break;
            }
        }
    }

    // Method to save game to json file that saves all the current states.
    void saveGame(const string & filename) {

        string adjustedFilename = filename;
        if (adjustedFilename.find(".json") == string::npos) {
            adjustedFilename += ".json";
        }

        json gameState;

        gameState["player"]["currentRoom"] = player.currentRoom;
        gameState["player"]["inventory"] = player.inventory;
        gameState["player"]["health"] = player.health;

        // Saving the state of each room in the game.
        for (const auto & [roomId, room]: rooms) {
            gameState["rooms"][roomId] = json {
                {
                    "id",
                    roomId
                }, {
                    "desc",
                    room.desc
                }, {
                    "altDesc",
                    room.altDesc
                }, {
                    "cctv_desc",
                    room.cctvDesc
                }, {
                    "validDesc",
                    room.validDesc
                }, {
                    "useAltDesc",
                    room.useAltDesc
                }, {
                    "isTeleportRoom",
                    room.isTeleportRoom
                }, {
                    "exits",
                    room.exits
                }, {
                    "status",
                    room.status
                }, {
                    "keyRequired",
                    room.keyRequired
                }
            };
        }
        // Saving the state of each object in the game.
        for (const auto & [objectId, object]: objects) {
            gameState["objects"][objectId] = json {
                {
                    "id",
                    objectId
                }, {
                    "desc",
                    object.desc
                }, {
                    "initialRoom",
                    object.initialRoom
                }, {
                    "requiredCode",
                    object.requiredCode
                }, {
                    "healthScore",
                    object.healthScore
                }, {
                    "isAvailable",
                    object.isAvailable
                }, {
                    "isShield",
                    object.isShield
                }, {
                    "isUsable",
                    object.isUsable
                }, {
                    "isOpenable",
                    object.isOpenable
                }, {
                    "isBreakable",
                    object.isBreakable
                }, {
                    "discardAfterUse",
                    object.discardAfterUse
                }, {
                    "requiresKey",
                    object.requiresKey
                }, {
                    "contents",
                    object.contents
                }, {
                    "breakBy",
                    object.breakBy
                }, {
                    "displayRooms",
                    object.displayRooms
                }
            };
        }
        // Saving the state of each enemy in the game.
        for (const auto & [enemyId, enemy]: enemies) {
            gameState["enemies"][enemyId] = json {
                {
                    "id",
                    enemyId
                }, {
                    "desc",
                    enemy.desc
                }, {
                    "aggressiveness",
                    enemy.aggressiveness
                }, {
                    "killedBy",
                    enemy.killedBy
                }, {
                    "initialRoom",
                    enemy.initialRoom
                }, {
                    "introMsg",
                    enemy.introMsg
                }, {
                    "successfulKillMsg",
                    enemy.successfulKillMsg
                }, {
                    "unsuccessfulKillMsg",
                    enemy.unsuccessfulKillMsg
                }, {
                    "successfulEscapeMsg",
                    enemy.successfulEscapeMsg
                }, {
                    "unsuccessfulEscapeMsg",
                    enemy.unsuccessfulEscapeMsg
                }, {
                    "requiredAnswer",
                    enemy.requiredAnswer
                }, {
                    "onDefeatUpdateRoom",
                    enemy.onDefeatUpdateRoom
                }, {
                    "onDefeatUpdateObject",
                    enemy.onDefeatUpdateObject
                }
            };
        }

        gameState["objective"] = gameObjective;

        std::ofstream outFile(adjustedFilename);
        if (!outFile) {
            cout << "Error: Unable to create or write to file: " << adjustedFilename << endl;
            return;
        }

        outFile << gameState.dump(4);
        cout << "Current game state has been saved into " << adjustedFilename << "." << endl;
    }

    // Method to load saved game state json file.
    void loadGame(const string & filename) {
        string adjustedFilename = filename;
        if (adjustedFilename.find(".json") == string::npos) {
            adjustedFilename += ".json";
        }

        std::ifstream inFile(adjustedFilename);
        if (!inFile) {
            cout << "Error: Unable to open file: " << adjustedFilename << endl;
            return;
        }

        json gameState;
        try {
            inFile >> gameState;
        } catch (json::parse_error & e) {
            cout << "Error: Invalid JSON in file: " << adjustedFilename << endl;
            return;
        }

        player.currentRoom = gameState["player"]["currentRoom"];
        player.inventory = gameState["player"]["inventory"].get < vector < string >> ();
        player.health = gameState["player"]["health"];

        for (const auto & [roomId, roomData]: gameState["rooms"].items()) {
            Room & room = rooms[roomId];
            room.id = roomId;
            room.desc = roomData["desc"];
            room.altDesc = roomData["altDesc"];
            room.cctvDesc = roomData["cctv_desc"];
            room.validDesc = roomData["valid_desc"];
            room.useAltDesc = roomData["useAltDesc"];
            room.isTeleportRoom = roomData["isTeleportRoom"];
            room.exits = roomData["exits"].get < map < string, string >> ();
            room.status = roomData["status"];
            room.keyRequired = roomData["keyRequired"];
        }

        for (const auto & [objectId, objectData]: gameState["objects"].items()) {
            Object & object = objects[objectId];
            object.id = objectId;
            object.desc = objectData["desc"];
            object.initialRoom = objectData["initialRoom"];
            object.requiredCode = objectData["requiredCode"];
            object.healthScore = objectData["healthScore"];
            object.isAvailable = objectData["isAvailable"];
            object.isShield = objectData["isShield"];
            object.isUsable = objectData["isUsable"];
            object.isOpenable = objectData["isOpenable"];
            object.isBreakable = objectData["isBreakable"];
            object.discardAfterUse = objectData["discardAfterUse"];
            object.requiresKey = objectData["requiresKey"];
            object.contents = objectData["contents"].get < vector < string >> ();
            object.breakBy = objectData["breakBy"].get < vector < string >> ();
            object.displayRooms = objectData["displayRooms"].get < vector < string >> ();
        }

        for (const auto & [enemyId, enemyData]: gameState["enemies"].items()) {
            Enemy & enemy = enemies[enemyId];
            enemy.id = enemyId;
            enemy.desc = enemyData["desc"];
            enemy.aggressiveness = enemyData["aggressiveness"];
            enemy.killedBy = enemyData["killedBy"].get < vector < string >> ();
            enemy.initialRoom = enemyData["initialRoom"];
            enemy.introMsg = enemyData["introMsg"];
            enemy.successfulKillMsg = enemyData["successfulKillMsg"];
            enemy.unsuccessfulKillMsg = enemyData["unsuccessfulKillMsg"];
            enemy.successfulEscapeMsg = enemyData["successfulEscapeMsg"];
            enemy.unsuccessfulEscapeMsg = enemyData["unsuccessfulEscapeMsg"];
            enemy.requiredAnswer = enemyData["requiredAnswer"];
            enemy.onDefeatUpdateRoom = enemyData["onDefeatUpdateRoom"].get < map < string, string >> ();
            enemy.onDefeatUpdateObject = enemyData["onDefeatUpdateObject"].get < map < string, bool >> ();
        }

        cout << filename << " has been loaded." << endl;
        cout << " " << endl;

        gameObjective = gameState["objective"];

        displayRoomDetails();

    }
    // Method to process and validate all player commands.
    private: void processCommand(const string & command) {
        bool validAction = false;
        bool triggersEnemyAction = true;

        if (command == "quit" || command == "exit") {
            cout << "Do you want to exit the game? [y/n]: ";
            string response;
            getline(cin, response);
            transform(response.begin(), response.end(), response.begin(), ::tolower);
            if (response == "y"
                or response == "yes") {
                cout << "You have exited the game." << endl;
                exit(0);
            } else {
                cout << "Continue playing." << endl;
                return;
            }
        }

        auto intercomIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.id == "intercom" &&
                    enemyPair.second.initialRoom == player.currentRoom;
            });

        if (intercomIt != enemies.end()) {
            if (command.substr(0, 6) == "answer") {
                if (command.length() == 6) {
                    cout << "Answer what?" << endl;
                    cout << intercomIt -> second.unsuccessfulKillMsg << "You are attacked by intercom! You lost " <<
                        intercomIt -> second.aggressiveness << " health points." << endl;
                    player.adjustHealth(-intercomIt -> second.aggressiveness);

                    if (player.health <= 0) {
                        gameOver("You have been defeated! Game Over.");
                    } else {
                        cout << "Health remaining: " << player.health << endl;
                    }
                } else {
                    string playerAnswer = command.substr(7);
                    transform(playerAnswer.begin(), playerAnswer.end(), playerAnswer.begin(), ::tolower);
                    if (!handleIntercom(intercomIt -> second, playerAnswer)) {
                        cout << "Health remaining: " << player.health << endl;
                    }
                }
            } else {

                cout << intercomIt -> second.unsuccessfulEscapeMsg << "You are attacked by intercom! You lost " <<
                    intercomIt -> second.aggressiveness << " health points." << endl;
                player.adjustHealth(-intercomIt -> second.aggressiveness);

                if (player.health <= 0) {
                    gameOver("You have been defeated! Game Over.");
                } else {
                    cout << "Health remaining: " << player.health << endl;
                }
            }
            return;
        }

        if (command.substr(0, 2) == "go") {
            if (command.length() > 3) {
                string direction = command.substr(3);
                validAction = movePlayer(direction);
                validAction = true;
            } else {
                cout << "Go where?" << endl;
            }
        } else if (command == "save") {
            string filename;
            cout << "Enter filename to save game: ";
            getline(cin, filename);
            saveGame(filename);
        } else if (command == "load") {
            string filename;
            cout << "Enter filename to load game: ";
            getline(cin, filename);
            loadGame(filename);
        } else if (command.substr(0, 4) == "open") {
            if (command.length() > 5) {
                string objectName = command.substr(5);
                openObject(objectName);
                validAction = true;
            } else {
                cout << "Open what?" << endl;
            }
        } else if (command.substr(0, 5) == "break") {
            if (command.length() > 6) {
                string objectName = command.substr(6);
                breakObject(objectName);
                validAction = true;
            } else {
                cout << "Break what?" << endl;
            }
        } else if (command.substr(0, 4) == "take") {
            if (command.length() > 5) {
                string objectName = command.substr(5);
                takeObject(objectName);
                validAction = true;
            } else {
                cout << "Take what?" << endl;
            }
        } else if (command.substr(0, 3) == "eat") {
            if (command.length() > 4) {
                string objectName = command.substr(4);
                eatObject(objectName);
                validAction = true;
            } else {
                cout << "Eat what?" << endl;
            }
        } else if (command == "list items" || command == "list") {
            listInventory();
            validAction = true;
            triggersEnemyAction =
                false;
        } else if (command.substr(0, 4) == "kill") {
            if (command.length() > 5) {
                string enemyName = command.substr(5);
                validAction = killEnemy(enemyName);
                if (unsuccessfulKillAttempt) {
                    unsuccessfulKillAttempt = false;
                    return;
                }
            } else {
                cout << "Kill what?" << endl;
            }
        } else if (command.substr(0, 4) == "look") {
            if (command == "look" || command == "look around") {
                displayRoomDetails();
                validAction = true;
                triggersEnemyAction = false;
            } else {
                string target = command.substr(5);
                if (target.empty()) {
                    cout << "Look at what?" << endl;
                } else if (objects.find(target) != objects.end() ||
                    find(player.inventory.begin(), player.inventory.end(), target) != player.inventory.end()) {
                    lookAtObject(target);
                    validAction = true;
                    triggersEnemyAction = false;
                } else if (enemies.find(target) != enemies.end()) {
                    lookAtEnemy(target);
                    validAction = true;
                    triggersEnemyAction = false;
                } else {
                    cout << "There is no " << target << " to look at." << endl;
                }
            }
        } else if (command.substr(0, 3) == "use") {
            if (command.length() > 4) {
                string objectName = command.substr(4);
                useObject(objectName);
                validAction = true;
            } else {
                cout << "Use what?" << endl;
            }
        } else {
            cout << "You thought you discovered a new command? \"" << command << "\" is not valid!" << endl;
        }

        if (validAction && triggersEnemyAction) {
            if (!player.justMoved) {
                enemyAction();
            }
        }

        player.justMoved = false;
    }
    // Method to handle actions performed by enemies in the game.
    void enemyAction() {
        auto enemyIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.initialRoom ==
                    player.currentRoom;
            });

        if (enemyIt != enemies.end() && enemyIt -> second.aggressiveness > 0) {
            enemyAttack(enemyIt -> second);
        }
    }
    // Method to handle player interactions with the 'intercom'.
    bool handleIntercom(Enemy & intercom,
        const string & playerAnswer) {
        if (playerAnswer == intercom.requiredAnswer) {
            cout << intercom.successfulKillMsg << endl;
            intercom.initialRoom = "";

            for (const auto & [objectId, availability]: intercom.onDefeatUpdateObject) {
                objects[objectId].isAvailable = availability;
                objects[objectId].initialRoom = player.currentRoom;
            }

            return true;
        } else {
            cout << intercom.unsuccessfulKillMsg << "You are attacked by intercom! You lost " <<
                intercom.aggressiveness << " health points." << endl;
            player.adjustHealth(-intercom.aggressiveness);

            if (player.health <= 0) {
                gameOver("You have been defeated! Game Over.");
                return false;
            }
            return false;
        }
    }

    // Method to move the player in a specified direction.
    bool movePlayer(const string & direction) {
        auto & currentRoom = rooms[player.currentRoom];
        auto it = currentRoom.exits.find(direction);

        if (it != currentRoom.exits.end()) {
            string nextRoomId = it -> second;
            Room & nextRoom = rooms[nextRoomId];

            if (nextRoom.isTeleportRoom) {
                srand(time(nullptr)); // Random number generator.

                // Check if the room is eligible for teleport (not teleport room, not hidden, not locked).
                vector < string > eligibleRoomIds;
                for (const auto & roomPair: rooms) {
                    if (!roomPair.second.isTeleportRoom && roomPair.second.status != "hidden" && roomPair.second.status != "locked") {
                        eligibleRoomIds.push_back(roomPair.first);
                    }
                }

                int randomIndex = rand() % eligibleRoomIds.size();
                nextRoomId = eligibleRoomIds[randomIndex];

                cout << "You have been teleported!" << endl;
            }

            if (nextRoom.status == "hidden") {
                cout << "You can't seem to find a way to go there." << endl;
                return false;
            }

            // Check if the room is locked and requires a key. Prevents entrance without it.
            if (nextRoom.status == "locked") {
                auto keyIt = find(player.inventory.begin(), player.inventory.end(), nextRoom.keyRequired);
                if (keyIt != player.inventory.end()) {
                    cout << "Using the " << * keyIt << " to unlock." << endl;
                    nextRoom.status = "unlocked";
                } else {
                    cout << "The door is locked. You need a " << nextRoom.keyRequired << " to unlock it." << endl;
                    return false;
                }
            }
            // Specific check to handle features in map6.json.
            if (rooms.find("cellar") != rooms.end() && nextRoomId == "bluebox") {
                if (find(player.inventory.begin(), player.inventory.end(), "key") == player.inventory.end()) {
                    cout << "You need a Yale key to enter the blue box." << endl;
                    return false;
                }
                if (statueBroken) {
                    cout << "The statue from the cellar attacks you as you try to open the blue box!" << endl;
                    gameOver("You have been defeated by the statue! Game Over.");
                    return false;
                }
            } else {
                // Handling enemies aggressiveness for chance of escape.      
                auto enemyIt = find_if(enemies.begin(), enemies.end(),
                    [ & ](const pair < string, Enemy > & enemyPair) {
                        return enemyPair.second.initialRoom == player.currentRoom;
                    });

                if (enemyIt != enemies.end()) {
                    int escapeChance = generateRandomNumber(0, 100);

                    if (escapeChance <= enemyIt -> second.aggressiveness) {
                        cout << (enemyIt -> second.unsuccessfulEscapeMsg.empty() ? "You failed to escape!" : enemyIt -> second.unsuccessfulEscapeMsg) << endl;
                        return false;
                    } else {
                        cout << (enemyIt -> second.successfulEscapeMsg.empty() ? "You managed to escape successfully!" : enemyIt -> second.successfulEscapeMsg) << endl;
                    }
                }

            }

            player.currentRoom = nextRoomId;
            player.justMoved = true;
            if (!nextRoom.isTeleportRoom) {
                cout << "Moving " << direction << endl;
            }
            displayRoomDetails();
            return true;

        } else {
            cout << "You can't go " << direction << endl;
            return false;
        }
    }

    // Method to handle the player use of objects in the game.
    void useObject(const string & objectName) {
        auto & obj = objects[objectName];

        if (!obj.isUsable) {
            cout << "The " << objectName << " is not usable in this way." << endl;
            return;
        }

        if (objectName == "monitor") {
            cout << "Viewing monitor..." << endl;
            for (const auto & roomId: obj.displayRooms) {
                if (rooms.find(roomId) != rooms.end()) {
                    cout << rooms[roomId].cctvDesc << endl;
                }
            }
        } else if (obj.requiredCode > 0) {
            cout << "Enter the code for " << objectName << ": ";
            string input;
            getline(cin, input);

            try {
                int userCode = stoi(input); // Try to convert the input string to an integer

                if (userCode == obj.requiredCode) {
                    cout << "Correct code! ";

                    for (const auto & [roomId, updates]: obj.onSuccessActions) {
                        for (const auto & [key, value]: updates) {
                            if (key == "desc") {
                                rooms[roomId].desc = value;
                            } else if (key == "exits") {
                                for (const auto & [exitName, exitTarget]: value.items()) {
                                    rooms[roomId].exits[exitName] = exitTarget;
                                }
                            } else if (key == "status") {
                                rooms[roomId].status = value;
                            }
                        }

                        cout << rooms[roomId].validDesc << endl;
                    }
                } else {
                    cout << "Incorrect code." << endl;
                }
            } catch (const invalid_argument & e) {
                cout << "Please enter a numerical code." << endl;
            }
        } else {
            cout << "The " << objectName << " does not have a specific use." << endl;
        }
    }

    // Method to handle the player's action of taking an object.
    void takeObject(const string & objectName) {

        // Check if the object is an enemy in the same room.
        auto enemyIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.id == objectName &&
                    enemyPair.second.initialRoom == player.currentRoom;
            });

        if (enemyIt != enemies.end()) {
            cout << "You can't take the " << objectName << "." << endl;
            return;
        }

        // Check if the object is available and in the current room
        auto objIt = find_if(objects.begin(), objects.end(),
            [ & ](const pair < string, Object > & obj) {
                return obj.second.id == objectName &&
                    obj.second.initialRoom == player.currentRoom &&
                    obj.second.isAvailable;
            });

        // Check if the object is usable or openable and prevent taking it, otherwise add object to player's inventory and mark as no longer available in the room.
        if (objIt != objects.end()) {
            if (objIt -> second.isUsable || objIt -> second.isOpenable || objIt -> second.isBreakable) {
                cout << "You can't take the " << objectName << " as it is not meant to be taken." << endl;
                return;
            }
            player.inventory.push_back(objIt -> first);
            objIt -> second.initialRoom = ""; // Object is now with the player
            cout << "You have taken the " << objectName << "." << endl;
        } else {
            cout << "There is no " << objectName << " here to take." << endl;
        }
    }

    // Method to handle the player's action of opening an object.
    void openObject(const string & objectName) {

        //Find the object within the current room that can be opened
        auto objIt = find_if(objects.begin(), objects.end(),
            [ & ](const pair < string, Object > & obj) {
                return obj.second.id == objectName &&
                    obj.second.initialRoom == player.currentRoom &&
                    obj.second.isOpenable;
            });

        if (objIt != objects.end()) {
            // Check if the object requires a key and if the player has it to open.
            if (!objIt -> second.requiresKey.empty()) {
                auto keyIt = find(player.inventory.begin(), player.inventory.end(), objIt -> second.requiresKey);
                if (keyIt == player.inventory.end()) {
                    cout << "It's locked. You need something to open it." << endl;
                    return;
                } else {
                    cout << "You use the " << objIt -> second.requiresKey << " to open the " << objectName << "." << endl;
                }
            }

            // Reveal the contents inside of the object to the player.
            for (const auto & contentId: objIt -> second.contents) {
                objects[contentId].isAvailable = true;
                objects[contentId].initialRoom = player.currentRoom;
                cout << "Inside, you find a " << objects[contentId].id << "." << endl;
            }
        } else {
            cout << "There is no " << objectName << " here to open." << endl;
        }
    }

    // Method to handle the player's action of breaking an object.
    void breakObject(const string & objectName) {

        // Check if the object exists in the same room.
        auto mapObjIt = objects.find(objectName);
        if (mapObjIt == objects.end()) {
            cout << "There is no " << objectName << " here to break." << endl;
            return;
        }

        // Check if the object is in the current room and breakable
        auto objIt = find_if(objects.begin(), objects.end(),
            [ & ](const pair < string, Object > & obj) {
                return obj.second.id == objectName &&
                    obj.second.initialRoom == player.currentRoom &&
                    obj.second.isBreakable;
            });

        if (objIt != objects.end()) {
            Object & objectToBreak = objIt -> second;

            // Check if player has the required item to break the object
            for (const auto & item: objectToBreak.breakBy) {
                if (find(player.inventory.begin(), player.inventory.end(), item) != player.inventory.end()) {
                    cout << "You use the " << item << " to break the " << objectName << "." << endl;

                    // Reveal contents after object is broken
                    for (const auto & contentId: objectToBreak.contents) {
                        objects[contentId].isAvailable = true;
                        objects[contentId].initialRoom = player.currentRoom;
                        cout << "Inside, you find a " << objects[contentId].id << "." << endl;
                    }
                    return;
                }
            }
            cout << "You can't break the " << objectName << " with the items you have." << endl;
        } else {
            cout << "The " << objectName << " is not breakable." << endl;
        }
    }

    // Method to display the list of items currently in the player's inventory.
    void listInventory() {
        if (player.inventory.empty()) {
            cout << "You are not carrying anything." << endl;
        } else {
            cout << "You are carrying:" << endl;
            for (const auto & item: player.inventory) {
                cout << "- " << objects[item].id << endl;
            }
        }
    }

    // Method to handle the player's action of eating an object. 
    void eatObject(const string & objectName) {
        // Check if player tries to eat object that is an enemy in the room.
        auto enemyIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.id == objectName &&
                    enemyPair.second.initialRoom == player.currentRoom;
            });

        if (enemyIt != enemies.end()) {
            cout << "You can't eat the " << objectName << "!" << endl;
            return;
        }

        // Proceed to check if it's a regular object that can be eaten in player's inventory.
        auto it = find_if(player.inventory.begin(), player.inventory.end(),
            [ & ](const string & itemId) {
                return itemId == objectName;
            });

        if (it != player.inventory.end()) {
            Object & obj = objects[ * it];
            if (obj.healthScore > 0) {
                cout << "Eating the " << obj.id <<
                    " will increase your health by " << obj.healthScore <<
                    ". Your current health is " << player.health << "." <<
                    "\nHealth cannot go over 100. Do you want to eat it? "
                "[y/n]: ";

                string response;
                getline(cin, response);
                transform(response.begin(), response.end(), response.begin(), ::tolower);

                if (response == "y"
                    or response == "yes") {
                    int oldHealth = player.health;
                    player.adjustHealth(obj.healthScore);

                    int healthIncrease = player.health - oldHealth;
                    cout << "You have eaten the " << obj.id <<
                        ". Health increased by " << healthIncrease <<
                        ". Health remaining: " << player.health << endl;

                    player.inventory.erase(it);
                } else {
                    cout << "You decided not to eat the " << obj.id << "." <<
                        endl;
                }
            } else {
                cout << "You can't eat " << obj.id << "." << endl;
            }
        } else {
            cout << "You don't have a " << objectName << "." << endl;
        }
    }

    // Method to handle the player's action of attempting to kill an enemy.
    bool killEnemy(const string & enemyName) {

        // Find the enemy in the current room that matches the enemyName.
        auto enemyIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.id == enemyName &&
                    enemyPair.second.initialRoom == player.currentRoom;
            });

        if (enemyIt != enemies.end()) {
            Enemy & enemy = enemyIt -> second;

            // Proceed to check if the player has all the items necessary to kill the enemy.
            bool canKill = all_of(enemy.killedBy.begin(), enemy.killedBy.end(),
                [ & ](const string & item) {
                    auto it = find(player.inventory.begin(), player.inventory.end(), item);
                    if (it != player.inventory.end()) {
                        // Check if the item should be discarded after use
                        if (objects[item].discardAfterUse) {
                            player.inventory.erase(it);
                            cout << "The " << item << " is used and discarded." << endl;
                        }
                        return true;
                    }
                    return false;
                });

            if (canKill) {
                cout << (enemy.successfulKillMsg.empty() ?
                        "You have killed the " + enemyName + "." :
                        enemy.successfulKillMsg) <<
                    endl;
                enemy.initialRoom = "";

                // Update room status or use alternative description after enemy is killed.
                for (const auto & [roomId, descOrStatus]: enemy.onDefeatUpdateRoom) {
                    if (descOrStatus == "useAltDesc") {
                        rooms[roomId].useAltDesc = true;
                    } else {
                        rooms[roomId].status = descOrStatus;
                    }
                }

                // Update object availability based on enemy defeat
                for (const auto & [objectId, availability]: enemy.onDefeatUpdateObject) {
                    objects[objectId].isAvailable = availability;
                }
                return true;
            } else {
                int damage = enemy.aggressiveness;
                player.adjustHealth(-damage);
                cout << (enemy.unsuccessfulKillMsg.empty() ?
                        "You attempted to kill the " + enemyName +
                        " and failed! You lost " +
                        to_string(damage) + " health points." :
                        enemy.unsuccessfulKillMsg) <<
                    " Health remaining: " << player.health << endl;
                if (player.health <= 0) {
                    gameOver("You have been defeated! Game Over.");
                }
                unsuccessfulKillAttempt = true;
                return false;
            }

            // Check if the broken enemy is the stone statue specifically to run map6.json.
            if (enemy.id == "stone statue") {
                statueBroken = true;
            }

        } else {
            cout << "There is no " << enemyName << " here." << endl;
            return false;
        }
    }

    // Method to handle the enemy's attack on the player.
    void enemyAttack(Enemy & enemy) {
        int damage = enemy.aggressiveness;

        // Check if the player has any shield in the inventory
        auto shieldIt = find_if(
            player.inventory.begin(), player.inventory.end(),
            [ & ](const string & itemId) {
                return objects[itemId].isShield;
            });

        // If player has shield in inventory use it to block the enemy attack and discard the shield after. Otherwise receive enemy damage.
        if (shieldIt != player.inventory.end()) {
            cout << "The " << enemy.id << " tries to attack you, but your " <<
                objects[ * shieldIt].id << " protects you!" << endl;
            cout << "The " << objects[ * shieldIt].id <<
                " is destroyed in the process." << endl;
            player.inventory.erase(
                shieldIt);
        } else if (damage > 0) {
            player.adjustHealth(-damage);
            cout << "You are attacked by " << enemy.id << "! You lost " <<
                damage << " health points. " <<
                "Health remaining: " << player.health << endl;

            if (player.health <= 0) {
                gameOver("You have been defeated by " + enemy.id +
                    "! Game Over.");
            }
        } else {
            // If enemy is present but with 0 aggressiveness.
            cout << "The " << enemy.id <<
                " is here but doesn't seem to attack right now." << endl;
        }
    }

    // Method to describe the player's current surroundings.
    void lookAround() {
        const auto & currentRoom = rooms[player.currentRoom];
        cout << currentRoom.desc << endl;

        // Iterate through all the objects in the same room as player and display those objects.
        for (const auto & obj: objects) {
            if (obj.second.initialRoom == player.currentRoom) {
                cout << "There is a " << obj.second.id << " here." << endl;
            }
        }

        // Iterate through all the enemies in the same room as player and display the enemies.
        auto enemyIt = enemies.find(player.currentRoom);
        if (enemyIt != enemies.end()) {
            cout << "There is a " << enemyIt -> second.id << " here." << endl;
        }
    }

    // Method to display detailed information about the player's current room.
    void displayRoomDetails() {
        const auto & currentRoom = rooms[player.currentRoom];
        string description = currentRoom.useAltDesc ? currentRoom.altDesc : currentRoom.desc;
        cout << description << endl;

        // Iterate through objects in the current player's room and display.
        for (const auto & pair: objects) {
            const Object & obj = pair.second;
            if (obj.initialRoom == player.currentRoom && obj.isAvailable) {
                cout << "There is a " << obj.id << " here." << endl;
            }
        }

        // Iterate through enemies in the current player's room and display.
        for (const auto & pair: enemies) {
            const Enemy & enemy = pair.second;
            if (enemy.initialRoom == player.currentRoom) {
                cout << (enemy.introMsg.empty() ? "There is a " + enemy.id + " here." : enemy.introMsg) << endl;
            }
        }
    }

    // Method to handle the player's action of looking at an object.
    void lookAtObject(const string & objectName) {
        // Check if the object is in the current room and available.	
        auto roomIt = find_if(objects.begin(), objects.end(),
            [ & ](const pair < string, Object > & obj) {
                return obj.second.id == objectName &&
                    obj.second.initialRoom == player.currentRoom &&
                    obj.second.isAvailable;
            });

        if (roomIt != objects.end()) {
            cout << roomIt -> second.desc << endl;
            return;
        }

        // Check if the object is in the player's inventory, display description if found
        auto inventoryIt = find_if(player.inventory.begin(), player.inventory.end(),
            [ & ](const string & itemId) {
                return itemId == objectName &&
                    objects[itemId].isAvailable;
            });

        if (inventoryIt != player.inventory.end()) {
            cout << objects[ * inventoryIt].desc << endl;
            return;
        }

        cout << "There is no " << objectName << " here or in your inventory, or it's not available to look at." << endl;
    }

    // Method to handle the player's action of looking at an enemy.
    void lookAtEnemy(const string & enemyName) {

        // Check if enemy in current room, if found display description. 
        auto enemyIt = find_if(enemies.begin(), enemies.end(),
            [ & ](const pair < string, Enemy > & enemyPair) {
                return enemyPair.second.id == enemyName &&
                    enemyPair.second.initialRoom == player.currentRoom;
            });

        if (enemyIt != enemies.end()) {
            cout << enemyIt -> second.desc << endl;
            return;
        }
        cout << "There is no enemy named '" << enemyName << "' here." << endl;
    }

    // Function to generate a random number within a specified range. Used to determine against enemy's aggressiveness if player can escape.
    int generateRandomNumber(int min, int max) {
        static random_device rd;
        static mt19937 rng(rd());
        uniform_int_distribution < int > uni(min, max);

        return uni(rng);
    }

    // Method to check if the player has met the winning conditions of the game.
    bool checkWinCondition() {
        string type = gameObjective["type"];
        auto what = gameObjective["what"].get < vector < string >> ();

        if (type == "kill") {
            for (const auto & enemyId: what) {
                auto enemyIt = enemies.find(enemyId);
                if (enemyIt != enemies.end() &&
                    enemyIt -> second.initialRoom != "") {
                    return false;
                }
            }
            return true;
        } else if (type == "collect") {
            for (const auto & itemId: what) {
                auto it = find(player.inventory.begin(), player.inventory.end(),
                    itemId);
                if (it == player.inventory.end()) {
                    return false;
                }
            }
            return true;
        } else if (type == "room") {
            for (const auto & roomId: what) {
                if (player.currentRoom != roomId) {
                    return false;
                }
            }
            return true;
        }

        return false;
    }
    // Method to handle the end of the game.
    void gameOver(const string & message) {
        cout << message << endl;
        exit(0);
    }
};

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <map_file.json>" << std::endl;
        return 1;
    }

    std::string filePath = argv[1];
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cout << "Error: Unable to open file or file does not exist. " << filePath << std::endl;
        return 1;
    }

    nlohmann::json gameData;
    inFile >> gameData;

    Game game(gameData);
    game.startGame();

    return 0;
}