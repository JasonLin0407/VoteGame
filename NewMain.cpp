#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <string>

// 注意：我們每回合都會給200000，所以如果你花100000拜票然後看到錢增加100000的話，這不是bug
// Style: https://google.github.io/styleguide/cppguide.html , 但enum用全大寫因為我們沒用macro，然後註解也不限長度
/*
待辦事項： (依照重要性排列)
  - 把文字的位置放好
  - 把文字的大小改好
  - !!!還有新聞台到底要幹嘛啦!!!
  - 把字體轉中文
  - 把round X 改成 距離選舉還有 kTotalNum - X 天
  - 增加賄賂選項並修改按下去的按鈕
  - 增加抹黑選項並修改按下去的按鈕
  - 增加輸入名字的環節

已完成：(依照時間排列)
  - 讓棋子出現在鍵盤上
  - 讓棋子依據dice而移動 (已經寫好取座標函數以及座標庫)

發現的Bug:(依照時間排列)

然後就是因為main有點大，不知道大家知不知道有一些text editor/ide 可以：
  - 把滑鼠停在某函數/變數上面即可看到註解 (前提是我有註解好)
  - 按control再把滑鼠停在某函數/變數上面可以看code內容，點進去可以直接跳到那個code的地方
  - 在寫出某函數/變數的地方ctrl+點擊滑鼠可以看到這個函數/變數被用在甚麼地方，然後可以直接改
  這些方法好像不是每個ide/text editor都這樣，但至少vscode 跟 visual studio可以
*/

enum State {  // 這告訴我們現在在哪個狀況下 然後main 就會跑相對應的程式
  MENU,
  NAME,  // 輸入名字 (目前還沒有)
  DICE,  // when the first player rolls the dice
  CITY,  // should be <location type 1> state_1
  NEWS,  // 當你走到新聞台
  WAIT,  // 做一個整理的動作
  END,
} state;

// Size consts
const int kWindowWidth = 1000;
const int kWindowHeight = 1000;
const int kPlayerWidth = 130;
const int kPlayerHeight = 130;

// Game Setting Consts
const int kPlayerNum = 2;
const int kTotalRounds = 10;
const int kLocationIndexNum = 28;  // num of total locations
const int kStartMoney = 20000000;
const int kBribeNum = 10;    // 遇到警察會回溯幾天看你有沒有賄選
const int kMiaoliNum = 3;    // 你會在苗栗卡幾天
const int kJailNum = 3;      // 你會在監獄卡幾天
const int kHospitalNum = 3;  // 你會在醫院卡幾天

// Const Earn Money Consts
const int kMoneyEachRound = 200000;      // 每回合增加多少錢
const int kMoneyEarnedAtStart = 500000;  // 一開始你拿到多少錢

// Const Spend Money Consts
const int kBaiPiaoSpendMoney = 100000;  // 拜票所花的錢

// Random Consts
const int kDiceMin = 1;
const int kDiceMax = 6;
const int kDiceRange = kDiceMax - kDiceMin + 1;
const int kBaiPiaoVoteMin = 5;                                                 // 拜票至少會讓你的選票百分比增加多少
const int kBaiPiaoVoteMax = 10;                                                // 拜票最多會讓你的選票百分比增加多少
const int kBaiPiaoVoteRange = kBaiPiaoVoteMax - kBaiPiaoVoteMin + 1;           // 拜票會讓你的選票百分比增加多少的範圍
const int kSpeechGetMoneyMin = 100000;                                         // 演講至少會讓你的選票百分比增加多少
const int kSpeechGetMoneyMax = 300000;                                         // 演講至多會讓你的選票百分比增加多少
const int kSpeechGetMoneyRange = kSpeechGetMoneyMax - kSpeechGetMoneyMin + 1;  // 演講會讓你的選票百分比增加多少的範圍

// Random Functions:
// 回傳隨機dice點數
int RandomDice() {
  return rand() % kDiceRange + kDiceMin;
}
// 回傳隨機拜票選票百分比數
int RandomBaiPiaoVote() {
  return rand() % kBaiPiaoVoteRange + kBaiPiaoVoteMin;
}
// 回傳隨機演講獲得的錢
int RandomSpeechGetMoney() {
  return rand() % kSpeechGetMoneyRange + kSpeechGetMoneyMin;
}

class Player {
 private:
  std::string name_;    // 名字
  int player_index_;    // 編號
  int location_index_;  // 我在哪
  int money_;
  int votes_;         // 這個應該用不到...
  int bribe_day_;     // if the player meets the police when bribe_day_ > 0, he gets caught
  int miaoli_day_;    // the days left until the player could go out of miaoli
  int jail_day_;      // the days left until the player could go out of jail
  int hospital_day_;  // the days left until the player could go out of hospital

 public:
  // Constructors and Destructors
  // - no Destructors, because no pointers exists.
  Player(std::string name, int player_index) {
    name_ = name;
    player_index_ = player_index;
    location_index_ = 0;
    money_ = kStartMoney;
    votes_ = 0;
    bribe_day_ = 0;
    miaoli_day_ = 0;
    jail_day_ = 0;
    hospital_day_ = 0;
  }

  // Accessors, Mutators, and Functions
  // Player name
  std::string get_player_name() const {
    return name_;
  }

  // Player Index
  int get_player_index() const {
    return player_index_;
  }

  // Location Index
  int get_location_index() const {
    return location_index_;
  }
  // 更新我在哪 輸入步數
  void UpdateLocationIndex(int steps) {
      location_index_ += steps;
    location_index_ %= kLocationIndexNum;
  }
  // 直接傳送到監獄
  void TeleportToJail() {
    location_index_ = 21;
  }

  // Money
  // returns the money of the player
  int get_money() const {
    return money_;
  }

  // enter the amount you want to add or spend. Amount is positive if add, and negative if spend.
  void UpdateMoney(int amount) {
    money_ += amount;
  }

  // Votes
  // plz dont use this rn
  int get_votes() const {
    return votes_;
  }

  // Bribe
  // pass in true when today is bribe day. Otherwise, just put this code when a day passess.
  void UpdateBribeDay(bool is_today_bribe_day = false) {
    if (is_today_bribe_day)
      bribe_day_ = kBribeNum;
    else
      bribe_day_ = std::max(bribe_day_ - 1, 0);
  }
  // When the police wants to check if you have bribed
  bool get_is_bribed() const {
    return (bribe_day_ > 0) ? true : false;
  }

  // miaoli
  // updates the days left until freedom
  void UpdateMiaoliDay(bool is_today_miaoli_day = false) {
    if (is_today_miaoli_day)
      miaoli_day_ = kMiaoliNum;
    else
      miaoli_day_ = std::max(miaoli_day_ - 1, 0);
  }
  // 回傳你是否卡在苗栗
  bool get_is_still_in_miaoli() const {
    return (miaoli_day_ > 0) ? true : false;
  }
  // 回傳你還剩下幾天出去苗栗
  int get_miaoli_day() const {
    return miaoli_day_;
  }

  // Jail
  // updates the days left until freedom
  void UpdateJailDay(bool is_today_jail_day = false) {
    if (is_today_jail_day)
      jail_day_ = kJailNum;
    else
      jail_day_ = std::max(jail_day_ - 1, 0);
  }
  // 回傳你是否還在監獄
  bool get_is_still_in_jail() const {
    return (jail_day_ > 0) ? true : false;
  }
  // 你還剩下幾天從監獄裡出來
  int get_jail_day() const {
    return jail_day_;
  }

  // Hospital
  // updates the days left until freedom
  void UpdateHospitalDay(bool is_today_hospital_day = false) {
    if (is_today_hospital_day)
      hospital_day_ = kHospitalNum;
    else
      hospital_day_ = std::max(hospital_day_ - 1, 0);
  }
  // 回傳你是否還在住院
  bool get_is_still_in_hospital() const {
    return (miaoli_day_ > 0) ? true : false;
  }
  // 回傳你還剩下幾天出院
  int get_hospital_day() const {
    return hospital_day_;
  }
};

class Location {
 private:
  const std::string name_;          // 名字
  const int votes_in_this_region_;  // 這個地區的總選票數
  int vote_[kPlayerNum];            // vote_[x] 代表玩家x在這個選區的佔票比例數
  int current_winner_;              // 目前的贏家

 public:
  // Constructor
  Location(std::string name, int votes_in_this_region) : name_(name), votes_in_this_region_(votes_in_this_region) {  // assuming 2 players
    current_winner_ = kPlayerNum;
    for (int i = 0; i < kPlayerNum; i++) {
      vote_[i] = 50;
    }
  }

  // Accessors
  // 回傳這個地區的總票數(可以拿的)
  int get_votes_in_this_region() const {
    return votes_in_this_region_;
  }
  // 回傳這個地區目前的贏家
  int get_current_winner() {  // assuming 2 players
    current_winner_ = (vote_[0] > vote_[1]) ? 0 : 1;
    return current_winner_;
  }

  // Member Functions
  // updates the money of the player and the votes of the player in this location if baipiao
  void BaiPiao(Player *player) {  // assuming only 2 players
    int delta_vote = RandomBaiPiaoVote();
    vote_[player->get_player_index()] += delta_vote;
    vote_[std::abs(1 - player->get_player_index())] -= delta_vote;
    player->UpdateMoney(-kBaiPiaoSpendMoney);
  }

  // updates the money of the plaer and the votes of the player in this location if speech
  void Speech(Player *player) {
    int delta_money = RandomSpeechGetMoney();
    player->UpdateMoney(delta_money);
  }
};

// Functions on building and modifying text
// Builds the text
void BuildText(sf::Text &text, const sf::Font &font, const sf::String &content, unsigned int size,
               const sf::Color &color, sf::Uint32 style, float x, float y) {
  text.setFont(font);
  text.setString(content);
  text.setCharacterSize(size);
  text.setFillColor(color);
  text.setStyle(style);
  text.setOrigin(floor(text.getLocalBounds().width) / 2, floor(text.getLocalBounds().height) / 2);
  text.setPosition(x, y);
}

void BuildText(sf::Text &text, const sf::Font &font, const std::wstring &content, unsigned int size,
               const sf::Color &color, sf::Uint32 style, float x, float y) {
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setStyle(style);
    text.setOrigin(floor(text.getLocalBounds().width) / 2, floor(text.getLocalBounds().height) / 2);
    text.setPosition(x, y);
}


// 更新dice的文字
void ChangeDiceText(sf::Text &text, const int dice_number) {
  std::string dice_string = "You rolled " + std::to_string(dice_number) + " steps";
  text.setFillColor(sf::Color::Red);
  text.setString(dice_string);
  text.setPosition(220, 600);
  text.setCharacterSize(32);
}

// 更新地點告知的文字
void ChangeTellLocationText(sf::Text &text, const int location_index, const std::string *list_of_locations) {
  std::string location_string = "You are now at " + list_of_locations[location_index];
  text.setFillColor(sf::Color::Red);
  text.setString(location_string);
  text.setPosition(220, 650);
  text.setCharacterSize(32);
}

// 更新"你還剩下幾天才能離開苗栗"
void ChangeMiaoliText(sf::Text &text, const int miaoli_day) {
  std::string miaoli_string = "You have " + std::to_string(miaoli_day) + " days until you leave miaoli.";
  text.setString(miaoli_string);
}

// 更新"你還剩下幾天才能出監牢"
void ChangeJailText(sf::Text &text, const int jail_day) {
  std::string jail_string = "You have " + std::to_string(jail_day) + " days until you leave Jail.";
  text.setString(jail_string);
}

// 更新"你還剩下幾天才能出院"
void ChangeHospitalText(sf::Text &text, const int hospital_day) {
  std::string hospital_string = "You have " + std::to_string(hospital_day) + " days until you leave the hospital.";
  text.setString(hospital_string);
}

// 更新"這是第幾回合"
void ChangeTellRoundText(sf::Text &text, const int round) {
  std::string round_string = std::to_string(kTotalRounds - round + 1);
  text.setString(round_string);
}

// 更新"現在是誰在玩" 以及 "他有多少錢"
void ChangeTellPlayerAndPropertiesText(sf::Text &text, const Player *player) {
  std::string p_and_p_string = "Player: " + player->get_player_name() + "\n" + std::to_string(player->get_money()) + " ntd";
  text.setString(p_and_p_string);
}


// function that finds the coordinates of players
sf::Vector2f GetLocation(int player_num, int location_index, float coordinates[][4]) {
  sf ::Vector2f result = {coordinates[location_index][player_num * 2] * 10 / 14, coordinates[location_index][player_num * 2 + 1] * 10 / 14};
  return result;
}

int main(int argc, char **argv) {
  sf::RenderWindow render_window(sf::VideoMode(kWindowWidth, kWindowHeight), "cute cattt", sf::Style::Titlebar | sf::Style::Close);
  sf::Event ev;

  // 目前隨便放的字體
  sf::Font big_font;
  sf::Font chinese;
  big_font.loadFromFile("fonts/HanaleiFill-Regular.ttf");

  // Menu setup

  // Menu Text
  sf::Text menu_text;
  BuildText(menu_text, big_font, "MENU", 100, sf::Color::White, sf::Text::Regular, 500, 328);
  sf::Text menu_sentence;
  BuildText(menu_sentence, big_font, "press \"Space\" to continue", 50, sf::Color::Magenta, sf::Text::Italic, 500, 508);

  // Dice setup
  // Dice Randomizer
  srand(time(0));
  int dice_value = 0;

  // Dice Prompt Text
  sf::Text dice_prompt_text;
  BuildText(dice_prompt_text, big_font, "Press \"Space\" to roll the dice!", 44, sf::Color::Blue, sf::Text::Bold, 500, 294);

  // Dice Text
  sf::Text dice_text;
  BuildText(dice_text, big_font, "", 25, sf::Color::Blue, sf::Text::Regular, 500, 250);

  // Miao Li Text (optional)
  sf::Text miaoli_text;
  BuildText(miaoli_text, big_font, "", 25, sf::Color::Blue, sf::Text::Regular, 500, 500);

  // Jail Text (optional)
  sf::Text jail_text;
  BuildText(jail_text, big_font, "", 25, sf::Color::Blue, sf::Text::Regular, 800, 500);

  // Hospital Text (optional)
  sf::Text hospital_text;
  BuildText(hospital_text, big_font, "", 25, sf::Color::Blue, sf::Text::Regular, 800, 500);

  // CITY and NEWS setup
  // Tell Location Text
  sf::Text tell_location_text;
  BuildText(tell_location_text, big_font, "", 25, sf::Color::Blue, sf::Text::Regular, 500, 500);

  // CITY setup
  // Option Bai Piao
  sf::Text option_bai_piao_text;
  BuildText(option_bai_piao_text, big_font, "1. pay 100 wan to get 5~10 percent of votes", 30, sf::Color::Blue, sf::Text::Regular, 500, 267);

  // Option Speech
  sf::Text option_speech_text;
  BuildText(option_speech_text, big_font, "2. get 10~30 wan ntd", 30, sf::Color::Blue, sf::Text::Regular, 500, 307);

  // WAIT setup
  sf::Text next_player_prompt;
  BuildText(next_player_prompt, big_font, "press \"Space\" to pass the dice \n to the next player!", 45 , sf::Color::Blue, sf::Text::Regular, 500, 294);

  // BackGround Setup

  //地圖
  sf::Texture board_texture;
  board_texture.create(kWindowWidth, kWindowHeight);
  board_texture.loadFromFile("images/map.png");
  sf::Sprite board_sprite;
  board_sprite.setTexture(board_texture);

  // Location Setup
  const std::string list_of_location_names_mandarin[kLocationIndexNum] = {
      "總統府",
      "基隆",
      "新北",
      "臺北",
      "桃園",
      "新聞台",
      "新竹",
      "苗栗國",
      "臺中",
      "南投",
      "彰化",
      "新聞台",
      "雲林",
      "嘉義",
      "臺大醫院",
      "臺南",
      "高雄",
      "新聞台",
      "澎湖",
      "屏東",
      "臺東",
      "綠島監獄",
      "宜蘭",
      "警察局",
      "花蓮",
      "金門",
      "馬祖",
      "新聞台"};

  const std::string list_of_location_names[kLocationIndexNum] = {
      "President's place",
      "ji long",
      "xin bei",
      "tai bei",
      "tao yuan",
      "xin wen tai",
      "xin zhu",
      "Miaoli Kingdom",
      "tai zhong",
      "nan tou",
      "zhang hua",
      "xin wen tai",
      "yun lin",
      "jia yi",
      "ntu hospital",
      "tai nan",
      "gao xiong",
      "xin wen tai",
      "peng hu",
      "ping dong",
      "tai dong",
      "lv dao jian yu",
      "yi lan",
      "POLICEEEE",
      "hua lian",
      "jin men",
      "ma zu",
      "xin wen tai"};

  const int list_of_location_votes[kLocationIndexNum] = {
      0, 37, 403, 262, 226, 0, 102, 0, 282, 49, 126, 0, 68, 77, 0, 187, 277, 0, 11, 81, 22, 0, 45, 0, 33, 14, 2, 0};
  Location *locations[kLocationIndexNum];
  for (int i = 0; i < kLocationIndexNum; i++) {
    locations[i] = new Location(list_of_location_names[i], list_of_location_votes[i]);
  }

  float coordinates[28][4] = {
      {1177, 1257, 1309, 1257},
      {1042, 1185, 1042, 1253},
      {893, 1185, 893, 1253},
      {744, 1185, 744, 1253},
      {601, 1185, 601, 1253},
      {450, 1211, 450, 1291},
      {304, 1185, 304, 1253},
      {57, 1217, 141, 1217},
      {100, 1057, 175, 1057},
      {100, 908, 175, 908},
      {100, 762, 175, 762},
      {61, 603, 127, 603},
      {100, 465, 175, 465},
      {100, 319, 175, 319},
      {49, 77, 149, 77},
      {304, 89, 304, 155},
      {450, 89, 450, 155},
      {601, 65, 601, 149},
      {744, 89, 744, 155},
      {813, 89, 813, 155},
      {1042, 89, 1042, 155},
      {1178, 77, 1318, 77},
      {1162, 319, 1239, 319},
      {1200, 465, 1284, 465},
      {1162, 603, 1239, 603},
      {1162, 762, 1239, 762},
      {1162, 908, 1239, 908},
      {1200, 1057, 1284, 1057}};

  // Player Setup
  std::string list_of_player_names[kPlayerNum] = {"one", "two"};
  Player *players[kPlayerNum];
  for (int i = 0; i < kPlayerNum; i++) {
    players[i] = new Player(list_of_player_names[i], i);  // 在沒有做輸入player1 2的名字前 先暫時這樣
  }

  sf::Texture cat_texture;
  cat_texture.create(kPlayerWidth, kPlayerHeight);
  cat_texture.loadFromFile("images/cat.png");
  sf::Sprite cat_sprite;
  cat_sprite.setTexture(cat_texture);
  cat_sprite.setScale((static_cast<float>(kPlayerWidth) / kWindowWidth), (static_cast<float>(kPlayerHeight) / kWindowHeight));
  cat_sprite.setPosition(GetLocation(0, 0, coordinates));

  sf::Texture prof_texture;
  prof_texture.create(kPlayerWidth, kPlayerHeight);
  prof_texture.loadFromFile("images/prof.png");
  sf::Sprite prof_sprite;
  prof_sprite.setTexture(prof_texture);
  prof_sprite.setScale((static_cast<float>(kPlayerWidth) / kWindowWidth), (static_cast<float>(kPlayerHeight) / kWindowHeight));
  prof_sprite.setPosition(GetLocation(1, 0, coordinates));

  // General text setup
  // tell round text
  sf::Text tell_round_text;
  tell_round_text.setOrigin(floor(tell_round_text.getLocalBounds().width) / 2, floor(tell_round_text.getLocalBounds().height) / 2);
  BuildText(tell_round_text, big_font, std::to_string(kTotalRounds), 96, sf::Color::Black, sf::Text::Regular, 660, 710);

  // tell player and properties text
  sf::Text tell_player_and_properties_text;
  std::string p_and_p_init = "Player: " + players[0]->get_player_name() +  "\n" + std::to_string(players[0]->get_money()) + " ntd";
  BuildText(tell_player_and_properties_text, big_font, p_and_p_init, 28, sf::Color::Black, sf::Text::Regular, 320, 750);

  // tell who is the winner(set the string later when the game ends)
  sf::Text winner_text;
  std::string winner_str;

  // Game Loop
  state = MENU;
  int current_id = 0;     // The current id of the player (0 or 1)
  int current_round = 0;  // Stop when player[1] finishes round kTotalRounds
  int zero = 0;           // total votes in the end of player[0]
  int one = 0;            // total votes in the end of player[1]
  int winner = kPlayerNum;
  while (render_window.isOpen()) {
    switch (state) {
      case MENU:
        while (render_window.pollEvent(ev)) {
          switch (ev.type) {
            case sf::Event::EventType::Closed:
              render_window.close();
              break;
            case sf::Event::EventType::KeyPressed:
              if (ev.key.code == sf::Keyboard::Space) {
                // 按下空格就繼續，準備丟dice
                state = DICE;
              }
              break;
          }
        }
        render_window.clear(sf::Color::Black);
        render_window.draw(menu_text);
        render_window.draw(menu_sentence);
        render_window.display();
        break;

      case DICE:
        while (render_window.pollEvent(ev)) {
          switch (ev.type) {
            case sf::Event::EventType::Closed:
              render_window.close();
              break;
            case sf::Event::EventType::KeyPressed:
              if (ev.key.code == sf::Keyboard::Space) {
                // Update all the special cases
                players[current_id]->UpdateMiaoliDay();
                players[current_id]->UpdateJailDay();
                players[current_id]->UpdateHospitalDay();
                players[current_id]->UpdateBribeDay();

                // 更新固定現實的提示(回合，玩家，錢)
                ChangeTellRoundText(tell_round_text, current_round + 1);
                ChangeTellPlayerAndPropertiesText(tell_player_and_properties_text, players[current_id]);

                // Check if matches any of the special cases.
                // If so, go to state = WAIT and use get_is_still_in_XXX to get the text (see if-else in wait -> render_window.draw)
                if (players[current_id]->get_is_still_in_miaoli() || players[current_id]->get_is_still_in_hospital() || players[current_id]->get_is_still_in_jail()) {
                  ChangeMiaoliText(miaoli_text, players[current_id]->get_miaoli_day());
                  ChangeJailText(jail_text, players[current_id]->get_jail_day());
                  ChangeHospitalText(hospital_text, players[current_id]->get_hospital_day());
                  state = WAIT;
                }

                // 每回合給錢
                players[current_id]->UpdateMoney(kMoneyEachRound);

                // throw the dice, update location and text
                int dice_value = RandomDice();
                std::cout << dice_value << std::endl;  // 輸出到terminal裡面 方便debug

                players[current_id]->UpdateLocationIndex(dice_value);
                int new_location_index = players[current_id]->get_location_index();  // indicates the location of the player in this round
                ChangeDiceText(dice_text, dice_value);
                ChangeTellLocationText(tell_location_text, new_location_index, list_of_location_names);
                if (current_id == 0){
                    cat_sprite.setPosition(GetLocation(0, new_location_index, coordinates));
                }
                else if (current_id == 1) {
                    prof_sprite.setPosition(GetLocation(1, new_location_index, coordinates));
                }

                // check if is on start(總統府). If so, add money
                if (new_location_index == 0 && current_round != 0) {
                  players[current_id]->UpdateMoney(kMoneyEarnedAtStart);
                  state = WAIT;
                }

                // check if is on 苗栗. If so, change state and stay for 3 days
                else if (new_location_index == 7) {
                  players[current_id]->UpdateMiaoliDay(true);
                  ChangeMiaoliText(miaoli_text, players[current_id]->get_miaoli_day());
                  state = WAIT;
                }

                // check if is on 臺大醫院. If so, change state and stay for 3 days.
                else if (new_location_index == 14) {
                  players[current_id]->UpdateHospitalDay(true);
                  ChangeHospitalText(hospital_text, players[current_id]->get_hospital_day());
                  state = WAIT;
                }

                // check if is on Police and has bribed in kBribeNum days
                // if so, teleport to jail and stay for 3 days
                else if (new_location_index == 23) {
                  if (players[current_id]->get_is_bribed()) {
                    players[current_id]->TeleportToJail();
                    players[current_id]->UpdateJailDay(true);
                    ChangeJailText(jail_text, players[current_id]->get_jail_day());
                    if (current_id == 0) {
                      cat_sprite.setPosition(GetLocation(0, 21, coordinates));
                    } else if (current_id == 1) {
                      prof_sprite.setPosition(GetLocation(1, 21, coordinates));
                    }

                  }
                  state = WAIT;
                }

                // else if at news
                else if (new_location_index == 5 || new_location_index == 11 || new_location_index == 17 || new_location_index == 27) {
                  state = NEWS;
                }
                // else at city
                else {
                  state = CITY;
                }
              }
              break;
          }
        }
        render_window.clear(sf::Color::Black);
        render_window.draw(board_sprite);
        render_window.draw(dice_prompt_text);
        render_window.draw(tell_round_text);
        render_window.draw(tell_player_and_properties_text);
        render_window.draw(cat_sprite);
        render_window.draw(prof_sprite);
        render_window.display();
        break;

      case CITY:
        while (render_window.pollEvent(ev)) {
          if (ev.type == sf::Event::EventType::Closed) {
            render_window.close();
          } else if (ev.type == sf::Event::EventType::KeyPressed) {
            switch (ev.key.code) {
              case sf::Keyboard::Escape:  // 按escape的話直接強迫結束
                state = END;
                break;
              case sf::Keyboard::Num1:
                // 呼叫拜票函數
                locations[players[current_id]->get_location_index()]->BaiPiao(players[current_id]);
                // 更新文字
                ChangeTellRoundText(tell_round_text, current_round + 1);
                ChangeTellPlayerAndPropertiesText(tell_player_and_properties_text, players[current_id]);
                state = WAIT;
                break;
              case sf::Keyboard::Num2:
                // 呼叫演說函數
                locations[players[current_id]->get_location_index()]->Speech(players[current_id]);
                // 更新文字
                ChangeTellRoundText(tell_round_text, current_round + 1);
                ChangeTellPlayerAndPropertiesText(tell_player_and_properties_text, players[current_id]);
                state = WAIT;
                break;
            }
          }
        }
        render_window.clear(sf::Color::Black);
        render_window.draw(board_sprite);
        render_window.draw(dice_text);
        render_window.draw(tell_round_text);
        render_window.draw(tell_player_and_properties_text);
        render_window.draw(tell_location_text);    // eg. "你現在在苗栗!"
        render_window.draw(option_bai_piao_text);  // eg. "1. 支付100萬，增取5~10%的選票"
        render_window.draw(option_speech_text);    // eg. "2. 獲得10~30萬台幣"
        render_window.draw(cat_sprite);
        render_window.draw(prof_sprite);
        render_window.display();
        break;

      case NEWS:
        while (render_window.pollEvent(ev)) {
          if (ev.type == sf::Event::EventType::Closed) {
            render_window.close();
          } else if (ev.type == sf::Event::EventType::KeyPressed) {
            switch (ev.key.code) {
              case sf::Keyboard::Escape:
                state = END;
                break;
              case sf::Keyboard::Space:
                state = WAIT;
                break;
            }
          }
        }
        render_window.clear(sf::Color::Black);
        render_window.draw(board_sprite);
        render_window.draw(tell_round_text);
        render_window.draw(tell_player_and_properties_text);
        render_window.draw(tell_location_text);  // eg. "你現在在新聞台!"
        render_window.draw(cat_sprite);
        render_window.draw(prof_sprite);
        render_window.display();
        break;

      case WAIT:
        while (render_window.pollEvent(ev)) {
          if (ev.type == sf::Event::EventType::Closed) {
            render_window.close();
          } else if (ev.type == sf::Event::EventType::KeyPressed) {
            switch (ev.key.code) {
              case sf::Keyboard::Space:
                std::cout << "round: " << current_round + 1 << std::endl;       // 方便debug
                std::cout << "rn is " << current_id << "'s turn" << std::endl;  // 方便debug
                // 如果是玩家1 (有兩個玩家：玩家0 與玩家1)的回合，那麼回合++
                if (current_id == kPlayerNum - 1) {
                  current_round++;
                }
                // 如果是玩家1 (有兩個玩家：玩家0 與玩家1)的回合，而且已經到限定的回合了，那麼結算選區，找出贏家
                if (current_id == kPlayerNum - 1 && current_round == kTotalRounds) {
                  // 每個選區跑一遍
                  for (int i = 0; i < kLocationIndexNum; i++) {
                    if (locations[i]->get_current_winner() == 0) {
                      zero += locations[i]->get_votes_in_this_region();
                    } else if (locations[i]->get_current_winner() == 1) {
                      one += locations[i]->get_votes_in_this_region();
                    }
                  }
                  // 找出贏家。先比對選票數，如果相同，再比較金錢
                  winner = (zero > one) ? 0 : 1;
                  if (zero == one) {
                    winner = (players[0]->get_money() > players[1]->get_money()) ? 0 : 1;
                  }
                  std::cout << "end game, winner: " << winner << std::endl;  // 方便debug
                  winner_str = winner == 0 ? "The winner is player one !" : "The winner is player two !";
                  BuildText(winner_text, big_font, winner_str, 45, sf::Color::White, sf::Text::Regular, 500, 800);
                  state = END;
                } else {
                  // 如果沒有結算的話，交換玩家，更新文字，回到dice
                  current_id = std::abs(1 - current_id);
                  ChangeTellRoundText(tell_round_text, current_round + 1);
                  ChangeTellPlayerAndPropertiesText(tell_player_and_properties_text, players[current_id]);
                  state = DICE;
                  break;
                }
            }
          }
        }
        render_window.clear(sf::Color::Black);
        render_window.draw(board_sprite);
        // 如果有特殊情況的話，把那個特殊情況印出來(文字在Dice那邊做修正)
        if (players[current_id]->get_is_still_in_miaoli()) {
          render_window.draw(miaoli_text);
        } else if (players[current_id]->get_is_still_in_jail()) {
          render_window.draw(jail_text);
        } else if (players[current_id]->get_is_still_in_hospital()) {
          render_window.draw(hospital_text);
        }
        render_window.draw(next_player_prompt);  // "press space to give the dice to the other player"
        render_window.draw(tell_round_text);
        render_window.draw(tell_player_and_properties_text);
        render_window.draw(cat_sprite);
        render_window.draw(prof_sprite);
        render_window.display();
        break;

      case END:
        while (render_window.pollEvent(ev)) {
          switch (ev.type) {
            case sf::Event::EventType::Closed:
              render_window.close();
              break;
          }
        }
        render_window.clear(sf::Color::Black);
        // Display the winner and the  winner's photo
        if (winner == 0){
            cat_sprite.setOrigin(floor(cat_sprite.getLocalBounds().width) / 2, floor(cat_sprite.getLocalBounds().height) / 2);
            cat_sprite.setPosition(kWindowWidth / 2, kWindowHeight / 2);
            cat_sprite.setScale(1.2, 1.2);
            render_window.draw(cat_sprite);
        }
        else if (winner == 1){
            prof_sprite.setOrigin(floor(prof_sprite.getLocalBounds().width) / 2, floor(prof_sprite.getLocalBounds().height) / 2);
            prof_sprite.setPosition(kWindowWidth / 2, kWindowHeight / 2);
            prof_sprite.setScale(1.2, 1.2);
            render_window.draw(prof_sprite);
        }
        render_window.draw(winner_text);// eg. "winner is XXX. \n <player0> has xx votes.\n <player1> has yy votes."
        render_window.display();
        break;
    }
  }

  // delete掉array of pointers
  for (int i = 0; i < kLocationIndexNum; i++) {
    delete locations[i];
    locations[i] = nullptr;
  }
  for (int i = 0; i < kPlayerNum; i++) {
    delete players[i];
    players[i] = nullptr;
  }

  return 0;
}
