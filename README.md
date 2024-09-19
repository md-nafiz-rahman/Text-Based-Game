# Installation Guide for Text Adventure Game

## Prerequisites

Before running the game, ensure the following are set up:

1. **Ubuntu or a C++ IDE**
   - You can run the game on Ubuntu or any C++ IDE.
   
2. **nlohmann/json.hpp**
   - This game uses the `nlohmann/json.hpp` file for JSON parsing. You can download it from the official GitHub repository: [nlohmann/json](https://github.com/nlohmann/json).
   - Make sure to include this header file in your project directory or ensure your IDE has access to it.

## Installation Steps

Follow these steps to install and run the game:

1. **Clone the repository**
   - Download the game files from the repository using the following command:
     ```bash
     git clone <repository-url>
     ```

2. **Navigate to the project directory**
   - Once the repository is cloned, move into the project directory:
     ```bash
     cd <project-directory>
     ```

3. **Ensure `nlohmann/json.hpp` is available**
   - Verify that the `json.hpp` file is in your project’s include directory or the same directory as the source files. If not, download it from the link above and place it in the appropriate location.

4. **Compile the game using the Makefile**
   - The game includes a Makefile for compiling. Run the following command to compile the game:
     ```bash
     make
     ```

5. **Run the game**
   - After compilation, run the game using the following command:
     ```bash
     ./main <map_file.json>
     ```
   - Replace `<map_file.json>` with the actual map file you want to use (e.g., `map1.json`).

## Example of Running the Game
To start the game using a specific map file:
```bash
./main map1.json
 ```

# Text Adventure Game - Command Guide

## Navigation Commands:
- **go \<direction\>**
  - **Description**: Move to a different room using the specified direction (e.g. east, west, up, down).
  - **Example**: `go east`

- **look / look around**
  - **Description**: View the description of the current room, its objects, and enemies (if any).
  - **Example**: `look`

## Object Interaction Commands:
- **take \<object_name\>**
  - **Description**: Pick up the specified object and carry it with you.
  - **Example**: `take key`

- **list items**
  - **Description**: Display a list of all items currently in your possession.
  - **Example**: `list items`

- **look \<object_name\>**
  - **Description**: View the description of a specific object in the room or in your possession.
  - **Example**: `look book`

- **use \<object_name\>**
  - **Description**: Interact with usable items or elements.
  - **Example**: `use computer`

## Enemy Interaction Commands:
- **kill \<enemy_name\>**
  - **Description**: Attempt to kill an enemy. The correct objects to defeat the enemy will be used automatically.
  - **Example**: `kill dragon`

## Game Control Commands:
- **exit**
  - **Description**: Exit the game after confirmation.
  - **Example**:
    ```bash
    > exit
    Do you want to exit the game? [y/n]: yes
    ```

- **save**
  - **Description**: Save the current game state to a file.
  - **Example**:
    ```bash
    > save
    Enter filename to save game: my_game
    ```

- **load**
  - **Description**: Load a previously saved game state from a file.
  - **Example**:
    ```bash
    > load
    Enter filename to load game: my_game
    ```

## New Map-Specific Commands:
- **open \<object_name\>**
  - **Description**: Open objects such as 'drawer' and 'cabinet'.
  - **Example**: `open drawer`

- **break \<object_name\>**
  - **Description**: Break certain objects, revealing hidden items.
  - **Example**: `break glass box`

- **answer \<your_answer\>**
  - **Description**: Provide an answer to a riddle posed by an enemy.
  - **Example**: `answer keyboard`
