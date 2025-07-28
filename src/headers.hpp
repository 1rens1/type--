// Import libraries
#include <fmt/color.h>
#include <fmt/core.h>

#include <algorithm>
#include <chrono>
#include <cpp-terminal/input.hpp>
#include <cpp-terminal/key.hpp>
#include <cpp-terminal/terminal.hpp>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

// Import word list
#include "words.hpp"

// Aliases for convenience
using std::string;
using std::vector;

//* Manipulation helpers

// Check if a vector contains a given value
template <typename T> bool includes(const vector<T> &vec, const T &value) {
  return std::find(vec.begin(), vec.end(), value) != vec.end();
}

// Splits every line of a string to a vector
vector<string> splitLines(const string &str) {
  std::istringstream ss(str);
  string line;
  vector<string> lines;
  while (std::getline(ss, line)) {
    lines.push_back(line);
  }
  return lines;
}

//* Displays
string generate_splash() {
  string splash = R"( _                                
| |_ _   _ _ __   ___   _     _   
| __| | | | '_ \ / _ \_| |_ _| |_ 
| |_| |_| | |_) |  __/_   _|_   _|
 \__|\__, | .__/ \___| |_|   |_|  
     |___/|_|             @rensdev)";

  vector<string> lines = splitLines(splash);
  const float freq = 0.2f;

  string result;

  for (size_t i = 0; i < lines.size(); ++i) {
    const auto &line = lines[i];

    for (size_t j = 0; j < line.length(); ++j) {
      char ch = line[j];

      // Generate RGB using a sine wave rainbow pattern
      int r = static_cast<int>(std::sin(freq * (j + i) + 0) * 127 + 128);
      int g = static_cast<int>(std::sin(freq * (j + i) + 2) * 127 + 128);
      int b = static_cast<int>(std::sin(freq * (j + i) + 4) * 127 + 128);

      result += fmt::format(fmt::fg(fmt::rgb(r, g, b)), "{}", ch);
    }
    result += "\r\n";
  }

  return result;
}

void print_splash() {
  static const string splash_output = generate_splash(); // Static to avoid re-generating
  fmt::print("{}", splash_output);
}

//* io

// Read a single key event
Term::Key get_key() {
  while (true) {
    const Term::Event event = Term::read_event();
    if (event.type() == Term::Event::Type::Key) {
      return Term::Key(event);
    }
  }
}

// Get a key only if it matches allowed keys
Term::Key get_whitelisted_key(const vector<Term::Key> &whitelist) {
  Term::Key key;
  do {
    key = get_key();
  } while (!includes(whitelist, key));
  return key;
}

// Move the cursor relative to current position
void move_cursor(int dx, int dy) {
  if (dy < 0)
    fmt::print("\x1b[{}A", -dy);
  if (dy > 0)
    fmt::print("\x1b[{}B", dy);
  if (dx > 0)
    fmt::print("\x1b[{}C", dx);
  if (dx < 0)
    fmt::print("\x1b[{}D", -dx);
}
// Set the cursor position
void set_cursor(int row, int col) { fmt::print("\033[{};{}H", row, col); }

// Clear current terminal line
void clear_line() {
  fmt::print("\r\x1b[2K");
  std::cout.flush();
}

// Clear entire screen
void clear_screen() { fmt::print("\033[2J\033[H"); }

const string prompt_prefix = "\u276f";
const size_t prompt_prefix_length = 1;

//* Time
long long now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

//* Game related
enum class PlayAction {
  Restart,
  GoToMainMenu,
};

// Generate a random integer between min and max (inclusive)
int random_int(int min, int max) {
  static std::random_device rd;  // non-deterministic seed
  static std::mt19937 gen(rd()); // Mersenne Twister engine
  std::uniform_int_distribution<> dist(min, max);
  return dist(gen);
}

// Generate a random string of words with the specified count from the word list
string generate_words(int word_count) {
  string result = "";

  int last_word_index;

  for (size_t i = 0; i < word_count; i++) {
    int word_index;
    do {
      word_index = random_int(0, words.size() - 1);
    } while (last_word_index == word_index); // Regenerate if duplicate

    result += words[word_index];
    if (i != word_count - 1)
      result += " ";

    last_word_index = word_index;
  }
  return result;
}
