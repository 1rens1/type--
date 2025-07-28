#include "headers.hpp"

// Display score, wpm, accuracy, etc
void display_score(int word_count, int elapsed_secs, int total_typed_chars, int correct_chars,
                   int incorrect_chars) {
  clear_screen();
  // Divided by 5 because it is the standard avarage word length
  float wpm = ((total_typed_chars - incorrect_chars) / 5.0) / (elapsed_secs / 60.0);
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow) | fmt::emphasis::bold |
                 fmt::emphasis::underline,
             "{:.1f}", wpm);
  fmt::print(" wpm (");
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "{}s", elapsed_secs);
  fmt::print(")\r\n\r\n");

  float accuracy = ((float)correct_chars / (float)total_typed_chars) * 100.0;
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "{:.1f}%", accuracy);
  fmt::print(" accuracy\r\n\r\n");

  float raw_wpm = (total_typed_chars / 5.0) / (elapsed_secs / 60.0);
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "{:.1f}", raw_wpm);
  fmt::print(" wpm (raw)\r\n\r\n");

  fmt::print("Letters: ");
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "{}", correct_chars);
  fmt::print(" correct ");
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "{}", incorrect_chars);
  fmt::print(" incorrect\r\n\r\n");

  fmt::print(fmt::emphasis::faint, "Press Enter to go to the main menu");

  // Can only continue if pressed Enter
  get_whitelisted_key({Term::Key::Enter});
}

PlayAction play(int word_count) {
  clear_screen();
  // Control hint
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "Esc");
  fmt::print(" Quit  ");
  fmt::print(fmt::fg(fmt::terminal_color::bright_yellow), "Tab");
  fmt::print(" Restart\r\n");

  fmt::print(fmt::emphasis::faint | fmt::emphasis::italic | fmt::emphasis::bold,
             "Start typing to begin...\r\n");

  // Generate random words by the specified amount
  string generated_words = generate_words(word_count);

  int cursor = 0;
  long long start_ms;
  long long total_typed_chars =
      0; // Whether it was correct, incorrect, or deleted (not including backspace)
  std::vector<bool> correctness; // true = correct, false = incorrect

  // Print the whole words for the test
  fmt::print(fmt::emphasis::faint, "{}\r", generated_words);
  // Ensure the cursor starts at row 1, col 3 to avoid word wrap issues.
  set_cursor(3, 1);

  // Repeat until the end of the words
  while (cursor < generated_words.length()) {
    auto key = get_key();

    if (key == Term::Key::Enter) {
      continue; // Ignore, go back to the top of the loop
    }
    if (key == Term::Key::Tab) {
      return PlayAction::Restart; // Restart the game
    }

    if (key == Term::Key::Esc) {
      return PlayAction::GoToMainMenu; // Go back to the main menu
    }

    // Start the timer when the first character is typed
    if (total_typed_chars == 0) {
      start_ms = now_ms();
      move_cursor(0, -1);
      clear_line();
      move_cursor(0, 1);
    };

    // Backspace handling
    if (key == Term::Key::Backspace) {
      if (cursor == 0)
        continue; // Ignore if at the start

      // Move cursor back, print the grayed character, then move cursor back again

      move_cursor(-1, 0);
      cursor--;
      fmt::print(fmt::emphasis::faint, string(1, generated_words[cursor]));
      move_cursor(-1, 0);

      if (!correctness.empty()) {
        correctness.pop_back(); // Remove the last correctness entry
      }

      continue;
    }

    char c = static_cast<char>(key);

    if (!std::isprint(key))
      continue; // Ignore non-printable characters (Ctrl+A, F1, etc.)

    char char_at_cursor = generated_words[cursor];

    total_typed_chars++;
    if (c != char_at_cursor) { // typed character is incorrect
      // Highlight with red
      fmt::print((char_at_cursor == ' ' ? fmt::bg : fmt::fg)(fmt::terminal_color::bright_red),
                 string(1, char_at_cursor));

      // Add incorrect entry to correctness
      correctness.push_back(false);
    } else { // typed character is correct
      fmt::print(string(1, char_at_cursor));

      // Add correct entry to correctness
      correctness.push_back(true);
    }
    // Move guide cursor (not actual cursor)
    cursor++;
  }

  long long elapsed_secs = (now_ms() - start_ms) / 1000;
  int correct_chars = std::count(correctness.begin(), correctness.end(), true); // Only count correct (true) characters
  int incorrect_chars = std::count(correctness.begin(), correctness.end(), false); // Only count incorrect (false) characters

  display_score(word_count, elapsed_secs, total_typed_chars, correct_chars, incorrect_chars);
  return PlayAction::GoToMainMenu;
}

int main() {
  vector<int> word_counts = {10, 25, 50, 100};
  int selected_word_count_index = 1;

  int running = true;
  while (running) {
    clear_screen();
    print_splash();
    fmt::print("\r\n");

    Term::Key main_menu_key;
    bool main_menu_running = true;
    while (main_menu_running) {
      fmt::print("  ");
      fmt::print(fmt::emphasis::faint, "[ \u2190 / \u2192 ] ");
      for (size_t i = 0; i < word_counts.size(); i++) {
        fmt::text_style style =
            selected_word_count_index == i
                ? fmt::fg(fmt::terminal_color::bright_yellow) | fmt::emphasis::bold
                : fmt::fg(fmt::terminal_color::yellow);
        fmt::print(style, "{}", word_counts[i]);
        if (i != word_counts.size() - 1)
          fmt::print(" ");
      }
      fmt::print(" Words\r\n\r\n  ");
      fmt::print(fmt::emphasis::faint, "[Space]");
      fmt::print(" Play      ");
      fmt::print(fmt::emphasis::faint, "[Esc]");
      fmt::print(" Exit\r\n");

      main_menu_key = get_whitelisted_key(
          {Term::Key::ArrowLeft, Term::Key::ArrowRight, Term::Key::Space, Term::Key::Esc});

      switch (main_menu_key) {
      case Term::Key::ArrowLeft: {
        selected_word_count_index =
            (selected_word_count_index + word_counts.size() - 1) % word_counts.size();
      } break;
      case Term::Key::ArrowRight: {
        selected_word_count_index = (selected_word_count_index + 1) % word_counts.size();
      } break;

      case Term::Key::Space: {
        main_menu_running = false;
        PlayAction action;
        do {
          action = play(word_counts[selected_word_count_index]);
        } while (action == PlayAction::Restart); // Will restart the game until action is not Restart
      } break;
      case Term::Key::Esc:
        running = false;
        main_menu_running = false;
        break;
      }
      // Clear the previous three lines to redraw
      for (size_t i = 0; i < 3; i++) {
        move_cursor(0, -1);
        clear_line();
      }
    }
  }
  return 0;
}
