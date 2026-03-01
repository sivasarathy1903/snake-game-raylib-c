# 🐍 Snake Game (Raylib + C)

A feature-rich and visually enhanced Snake Game built using **C and Raylib**.  
This project includes animated UI, sound effects, multiple difficulty levels, particle systems, and a complete game state architecture.

---

## 🎮 Game Features

### 🎬 Animated Title Screen
- Dynamic animated snake
- Floating star background
- Particle effects
- Smooth transition animations
- Background music

### 🎯 Gameplay
- Classic snake movement (W, A, S, D)
- Fruit collection system
- Score tracking
- Tail growth mechanics
- Collision detection (walls, tail, bombs)

### 💣 Obstacles
- Moving bombs
- Bomb repositioning every 5 seconds
- Game over on collision

### 🎚 Difficulty Levels
- Easy
- Medium
- Hard  
(Snake speed increases with difficulty)

### ⏸ Pause System
- Pause using `P` key
- Pause button (mouse click support)

### 🧠 Hint Screen
- Pre-game tips
- Countdown before gameplay starts

### 🏆 High Score System
- Top 5 scores stored during runtime
- Displayed in Game Over screen

### 🔊 Audio & Visual Effects
- Background music
- Sound effects (eat, click, game over)
- Animated buttons
- Gradient animated text
- Custom textures & backgrounds

---

## 🛠 Technologies Used

- **Language:** C
- **Graphics Library:** Raylib
- **Concepts Applied:**
  - Game State Management
  - Collision Detection
  - Particle Systems
  - Audio Streaming
  - Animation using Trigonometry
  - Structs & Enums
  - Texture Rendering
  - Real-time Input Handling

---

## 📂 Project Structure

```
Snake-Game-Raylib/
│── main.c
│── assets/
    ├── textures/
    ├── sounds/
    ├── music/
```

> ⚠ Note: Update asset file paths if running on a different system.

---

## ▶️ How to Run

### 1️⃣ Install Raylib

Make sure Raylib is installed.

For Windows (MinGW):
```
gcc main.c -o snake -lraylib -lopengl32 -lgdi32 -lwinmm
```

### 2️⃣ Run
```
./snake
```

---

## 🎮 Controls

| Action | Key |
|--------|------|
| Move Up | W |
| Move Down | S |
| Move Left | A |
| Move Right | D |
| Pause | P |
| Select Buttons | Mouse Click |

---

## 🎯 Learning Outcomes

- Built a complete 2D game using C
- Implemented game loop architecture
- Designed modular game state transitions
- Applied animation using `sinf()` and time-based updates
- Integrated audio with Raylib
- Designed custom UI components

---

## 🚀 Future Improvements

- Save high scores to file
- Add power-ups
- Add multiplayer mode
- Add mobile version
- Add leaderboard system

---

## 👨‍💻 Developers

- Sivasarathy A  
- Sudharsan B  
- Srikanth C  

---

🐍 *Built with creativity, logic, and a love for low-level programming.*
