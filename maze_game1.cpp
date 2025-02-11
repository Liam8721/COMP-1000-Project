#define NOMINMAX // prevents windows.h from defining min and max macros

// LIBRARIES
#include <iostream>
#include <cstdlib>
#include <stack>
#include <list>
#include <algorithm>
#include <thread>
#include <vector>
#include <windows.h>
#include <chrono>
#include <string>
#include <limits>
#include <fstream>

using namespace std;

// hash define to define specific integers and colour codes as words
#define MAZE_HEIGHT 19
#define MAZE_WIDTH 19
#define PLAYER_STARTING_X 1
#define PLAYER_STARTING_Y 1
#define PLAYER_ICON '8'
#define RED "\033[0;31m"
#define DEFAULT_COLOUR "\033[0m"
#define GREEN "\033[0;32m"
#define MAGENTA "\033[0;35m"
#define BLUE "\033[0;34m"

// GLOBAL VARIABLES

int player_x_position = PLAYER_STARTING_X, player_y_position = PLAYER_STARTING_Y, score = 0;

// customisable variables
int total_levels = 3, total_puzzles = 9, total_time = 1000, total_enemies = 3, total_swords = 1;

// variables that set up game
bool active_sword = false, game_end = false, lost = false, game_paused = false;
char player_mechanics, maze[MAZE_HEIGHT][MAZE_WIDTH];
int current_level = 1;

// puzzle variables
string puzzles[2] = {"math", "word"},
       words[8] = {"maze", "escape", "path", "open", "safe", "exit", "puzzle", "unlock"};
char operators[4] = {'+', '-', '/', '*'};

// enemy variables
int directions[4][2] = {
    {1, 0},  // down
    {0, 1},  // right
    {-1, 0}, // up
    {0, -1}, // left
};

stack<pair<int, int>> maze_path; // initialises stack for backtracking

// this creates the maze grid according to maze height and width and sets all cells to '1' except the starting point in preperation for path_generation()
void maze_template_creation()
{
    for (int y = 0; y < MAZE_HEIGHT; y++)
    {
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            maze[y][x] = '1';
        }
    }
    maze[PLAYER_STARTING_Y][PLAYER_STARTING_X] = '0';
}

// this function re-outputs the maze with the new locations of the player and enemies
void current_player_location()
{
    for (int y = 0; y < MAZE_HEIGHT; y++)
    {
        for (int x = 0; x < MAZE_WIDTH; x++)
        {
            switch ((int)maze[y][x])
            {
            case (int)'0':
                cout << BLUE; // makes each maze path blue
                break;
            case (int)'1':
                cout << DEFAULT_COLOUR; // makes the walls default colour
                break;
            case (int)'8':
                cout << MAGENTA; // makes player icon magenta
                break;
            case (int)'E':
                cout << RED; // makes enemy icon red
                break;
            case (int)'W':
                cout << GREEN; // makes exit to maze green
                break;
            case (int)'P':
                cout << RED; // makes all puzzle icons red
            }
            cout << maze[y][x] << " ";
        }
        cout << endl;
    }
}

// this generates a maze within the grid
void path_generation(int y, int x)
{
    // creation of random direction
    int random_direction = rand() % 4;
    int random_y = y + (directions[random_direction][0] * 2);
    int random_x = x + (directions[random_direction][1] * 2);

    // keeps track of the directions tried to ensure all directions are checked
    bool has_tried_up = false;
    bool has_tried_down = false;
    bool has_tried_right = false;
    bool has_tried_left = false;
    bool moveValid = false;

    // keeps track of the last coordinates to backtrack
    static int last_y;
    static int last_x;

    // loops through all directions to check if they are a valid direction to go in (within maze grid and unvisited)
    while ((has_tried_up == false) || (has_tried_down == false) || (has_tried_right == false) || (has_tried_left == false))
    {
        switch (random_direction)
        {
        case 0:
            has_tried_down = true;
            break;
        case 1:
            has_tried_right = true;
            break;
        case 2:
            has_tried_up = true;
            break;
        case 3:
            has_tried_left = true;
            break;
        }
        // if direction is within grid and marked unvisited, push coordinates to maze_path and mark the maze_path as visited
        if ((random_y >= 0 && random_y < MAZE_HEIGHT) && (random_x >= 0 && random_x < MAZE_WIDTH) && (maze[random_y][random_x] == '1'))
        {
            maze_path.push({random_y, random_x});
            maze[random_y][random_x] = '0';
            maze[y + directions[random_direction][0]][x + directions[random_direction][1]] = '0';

            last_y = random_y;
            last_x = random_x;

            path_generation(random_y, random_x);
            moveValid = true;
            break;
        }
        // if direction is not within grid or marked visited choose a new random direction
        else
        {
            random_direction = rand() % 4;
            random_y = y + (directions[random_direction][0] * 2);
            random_x = x + (directions[random_direction][1] * 2);
        }
    }

    // if there are no valid directions to move in then backtrack to the last valid coordinates and repeat the process
    if (moveValid == false)
    {
        // if the stack is empty then make the last coordinates the end of the maze othwerise backtrack
        if (maze_path.size() > 1)
        {
            maze_path.pop();
            int previous_y = maze_path.top().first;
            int previous_x = maze_path.top().second;
            path_generation(previous_y, previous_x);
        }
        else
        {
            maze[last_y][last_x] = 'W';
        }
    }
}

// this function generates puzzles around the maze where a wall exists
void puzzle_placement()
{
    vector<pair<int, int>> valid_coordinates;
    // loops through generated maze and saves coordinates of all valid spots for puzzles
    for (int y = 1; y < MAZE_HEIGHT - 1; y++)
    {
        for (int x = 1; x < MAZE_WIDTH - 1; x++)
        {
            if ((maze[y + 1][x] == '0' && maze[y - 1][x] == '0' && maze[y][x + 1] == '1' && maze[y][x - 1] == '1' && maze[y][x] == '1') ||
                (maze[y + 1][x] == '1' && maze[y - 1][x] == '1' && maze[y][x + 1] == '0' && maze[y][x - 1] == '0' && maze[y][x] == '1'))
            {
                valid_coordinates.push_back(make_pair(y, x));
            }
        }
    }

    for (int number = 0; number < total_puzzles; number++)
    {
        // choose random coordinate from valid directions vector and place puzzle relative to the total puzzles
        int random_index = rand() % valid_coordinates.size();
        pair<int, int> puzzle_position = valid_coordinates[random_index];
        maze[puzzle_position.first][puzzle_position.second] = 'P';
        valid_coordinates.erase(valid_coordinates.begin() + random_index);
    }
}

// this class enables all enemies to have the same structure and movement mechanics
class Enemy
{
public:
    // enemy variables
    int enemy_x_position;
    int enemy_y_position;
    int last_direction;
    bool enemy_active;
    void enemy_movement(int player_y_position, int player_x_position);

    Enemy(int y, int x) : enemy_y_position(y), enemy_x_position(x), last_direction(-1) {}
};
// this function moves the enemy towards the player
void Enemy::enemy_movement(int player_y_position, int player_x_position)
{
    // if enemy has been killed then return
    if (!enemy_active)
    {
        return;
    }
    int distance;

    distance = abs(enemy_x_position - player_x_position) + abs(enemy_y_position - player_y_position); // manhattan distance formula

    // initialising variables
    int up_distance, down_distance, left_distance, right_distance;
    int shortest_distance;
    list<int> distances_of_valid_directions;
    int static last_direction = -1;
    bool moved = false, enemy_active = true;

    // checks for collision
    if (distance != 1 && distance != 0)
    {
        // loops through all directions (up down left right)
        for (int counter = 0; counter < 4; counter++)
        {
            int temporary_y, temporary_x;
            temporary_y = enemy_y_position + (directions[counter][0]);
            temporary_x = enemy_x_position + (directions[counter][1]);

            if (last_direction != -1 && counter == (last_direction + 2) % 4)
            {
                continue;
            }

            // checks if direction is within bounds of maze grid and checks if new coordinates isnt a wall
            if ((temporary_y >= 0 && temporary_y < MAZE_HEIGHT) && (temporary_x >= 0 && temporary_x < MAZE_WIDTH) && (maze[temporary_y][temporary_x] == '0'))
            {
                // if condition is true then the distance is added onto the back of a list
                switch (counter)
                {
                case 0:
                    down_distance = abs((enemy_y_position + directions[0][0]) - player_y_position) + abs((enemy_x_position + directions[0][1]) - player_x_position);
                    distances_of_valid_directions.push_back(down_distance);
                    break;
                case 1:
                    right_distance = abs((enemy_y_position + directions[1][0]) - player_y_position) + abs((enemy_x_position + directions[1][1]) - player_x_position);
                    distances_of_valid_directions.push_back(right_distance);
                    break;
                case 2:
                    up_distance = abs((enemy_y_position + directions[2][0]) - player_y_position) + abs((enemy_x_position + directions[2][1]) - player_x_position);
                    distances_of_valid_directions.push_back(up_distance);
                    break;

                case 3:
                    left_distance = abs((enemy_y_position + directions[3][0]) - player_y_position) + abs((enemy_x_position + directions[3][1]) - player_x_position);
                    distances_of_valid_directions.push_back(left_distance);
                    break;
                }
            }

            // on the last loop the shortest distance is found in the loop
            if (counter == 3)
            {
                shortest_distance = *min_element(distances_of_valid_directions.begin(), distances_of_valid_directions.end());
            }
        }

        // loops through list
        for (int valid_distance : distances_of_valid_directions)
        {
            // compares each element in list (valid distances) to the shortest distance
            if (valid_distance == shortest_distance)
            {
                // compares distance to each direction to assess if statement is true
                // when direction and distance match then enemy moves in direction
                if (valid_distance == down_distance)
                {
                    last_direction = 0;
                    maze[enemy_y_position][enemy_x_position] = '0';
                    enemy_y_position = enemy_y_position + directions[0][0];
                    enemy_x_position = enemy_x_position + directions[0][1];
                    maze[enemy_y_position][enemy_x_position] = 'E';
                }

                else if (valid_distance == right_distance)
                {
                    last_direction = 1;
                    maze[enemy_y_position][enemy_x_position] = '0';
                    enemy_y_position = enemy_y_position + directions[1][0];
                    enemy_x_position = enemy_x_position + directions[1][1];
                    maze[enemy_y_position][enemy_x_position] = 'E';
                }

                else if (valid_distance == up_distance)
                {
                    last_direction = 2;
                    maze[enemy_y_position][enemy_x_position] = '0';
                    enemy_y_position = enemy_y_position + directions[2][0];
                    enemy_x_position = enemy_x_position + directions[2][1];
                    maze[enemy_y_position][enemy_x_position] = 'E';
                }

                else if (valid_distance == left_distance)
                {
                    last_direction = 3;
                    maze[enemy_y_position][enemy_x_position] = '0';
                    enemy_y_position = enemy_y_position + directions[3][0];
                    enemy_x_position = enemy_x_position + directions[3][1];
                    maze[enemy_y_position][enemy_x_position] = 'E';
                }
                moved = true;
                break; // ends for loop once distance and direction match
            }
        }

        last_direction = -1; // Reset last direction to allow new moves
    }
    // checks collision
    else
    {

        if (active_sword)
        {
            score += 50;
            this->enemy_active = false;
            // maze[enemy_y_position][enemy_x_position] = '0';
        }
        else
        {
            game_end = true;
            lost = true;
        }
    }
}

// gives user a puzzle once they land on a P and checks for correct input
bool puzzle_activated()
{
    cout << "\nYou have landed on a puzzle !!!\n";

    // if random puzzle option is 0 then player is given a math puzzle but if random puzzle option is 1 then player is given a word puzzle
    int random_puzzle_option = rand() % 2;
    if (puzzles[random_puzzle_option] == "math")
    {
        int first_number;
        int second_number;
        int random_operator = rand() % 4;
        int player_answer;

        // chooses operator randomly and ask user to solve question with 2 ranom numbers
        switch ((int)operators[random_operator])
        {
        case (int)'+':
            first_number = rand() % 11;
            second_number = rand() % 11;
            cout << first_number << " + " << second_number << " = ", "?";
            cin >> player_answer;
            if (!cin.fail())
            {
                if (player_answer == (first_number + second_number))
                {
                    cout << "\nCORRECT !!!";
                    cout << "\nYou have discovered a new path";
                return true;
                }
                else
                {
                    cout << "\nThe correct answer to " << first_number << "+" << second_number << "was " << first_number + second_number;
                    return false;
                }
            
            }
            else {
                cerr << "\nINVALID INPUT";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            break;

        case (int)'-':
            first_number = rand() % 11;
            second_number = rand() % 11;
            cout << first_number << " - " << second_number << " = " << "?";
            cin >> player_answer;
            if (!cin.fail())
            {
                if (player_answer == (first_number - second_number))
                {
                    cout << "\nCORRECT !!!";
                    cout << "\nYou have discovered a new path";
                    return true;
                }
                else
                {
                    cout << "\nThe correct answer to " << first_number << "-" << second_number << "was " << first_number - second_number;
                    return false;
                }
            }
            else {
                cerr << "\nINVALID INPUT";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            
            break;

        case (int)'*':
            first_number = rand() % 11;
            second_number = rand() % 11;
            cout << first_number << " * " << second_number << " = " << "?";
            cin >> player_answer;
            if (!cin.fail())
            {
                if (player_answer == (first_number * second_number))
                {
                    cout << "\nCORRECT !!!";
                    cout << "\nYou have discovered a new path";
                    return true;
                }
                else
                {
                    cout << "\nThe correct answer to " << first_number << "*" << second_number << "was " << first_number * second_number;
                    return false;
                }
            } 
            else {
                cerr << "\nINVALID INPUT";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            break;

        case (int)'/':
            int multiplier;
            second_number = (rand() % 10) + 1;
            multiplier = rand() % 10;
            first_number = second_number * multiplier;
            cout << first_number << " / " << second_number << " = " << "?";
            cin >> player_answer;
            if (cin.fail())
            {
                if (player_answer == (first_number / second_number))
                {
                    cout << "\nCORRECT !!!";
                    cout << "\nYou have discovered a new path";
                    return true;
                }
                else
                {
                    cout << "\nThe correct answer to " << first_number << "/" << second_number << "was " << first_number / second_number;
                    return false;
                }
            }
            else {
                cerr << "\nINVALID INPUT";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            break;

        default:
            break;
        }
    }

    else
    {
        // chooses random word from list and random letter from that word and changes it to an underscore to represent missing letter to fill in by player
        int random_number = rand() % 8;
        string random_word = words[random_number];
        int random_letter = rand() % (random_word.length());
        char player_answer, correct_letter = random_word[random_letter];
        random_word[random_letter] = '_';
        cout << random_word;
        cout << "\nEnter the missing letter here: ";
        cin >> player_answer;

        if (!cin.fail())
        {
            // converts player answer to lowercase to reduce any errors
            player_answer = tolower(player_answer);

            // if correct input then inform user and return true for the main function
            if (player_answer == correct_letter)
            {
                cout << "\nCORRECT !!!";
                cout << "\nYou have discovered a new path";
                return true;
            }
            // if incorrect input then inform user and return false for the main function
            else
            {
                random_word[random_letter] = correct_letter;
                cout << "\nINCORRECT !!!\n";
                cout << "\nThe correct answer was " << random_word;
                return false;
            }
        }
        else
        {
            cerr << "\nINVALID INPUT";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    return false; // this line will never be read as all possible outcomes produce return statement in the if statements however for good practice i left this here for code maintainability
}

// allows user to customise aspects of the game to their preferences
void settings()
{
    int option;
    string new_amount;
    bool end_settings = false;

    while (!end_settings)
    {
        // present user with text options
        cout << "\n============\n";
        cout << "  Settings  \n";
        cout << "============\n";
        cout << "\n";
        cout << "Customisable Options:\n";
        cout << "1. Number of levels\n";
        cout << "2. Timer\n";
        cout << "3. Number of enemies\n";
        cout << "4. Number of puzzles\n";
        cout << "5. Number of swords\n";
        cout << "6. Back to Main Menu\n";
        cout << "enter option: ";
        cin >> option;

        // checks for invalid input
        if (!cin.fail())
        {
            switch (option)
            {
            case 1:
                // edits number of total_levels
                while (true)
                {
                    cout << "\nYou have selected levels as your customisable option\n";
                    cout << "Please enter a new amount of levels you want to have in your game (must be between 0 and 4) or enter the letter 'b' to go back: ";
                    cin >> new_amount;

                    if (new_amount == "b")
                    {
                        cout << "back";
                        break;
                    }

                    else if (stoi(new_amount) > 0 && stoi(new_amount) < 5)
                    {
                        total_levels = stoi(new_amount);
                        cout << "\nThe amount of levels you have chosen is " << total_levels << "\n";
                        break;
                    }

                    // if user input invalid then inform user, clear cin to prevent erros and repeat options
                    else
                    {
                        cerr << "\nINVALID INPUT!!! Please enter one of the valid options \n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

                break;

            case 2:
                // edits timer
                while (true)
                {
                    cout << "\nYou have selected timer as your customisable option\n";
                    cout << "Please enter a new amount of seconds you want to have in each level or enter the letter 'b' to go back: ";
                    cin >> new_amount;
                    if (new_amount == "b")
                    {
                        cout << "\nyou have selected to go back\n";
                        break;
                    }

                    try
                    {
                        if (stoi(new_amount) >= 1)
                        {
                            total_time = stoi(new_amount);
                            cout << "\nThe new amount of seconds you have chosen is " << total_time << "\n";
                            break;
                        }
                        else
                        {
                            throw out_of_range("A timer cannot be less than 1");
                        }
                    }
                    catch (const invalid_argument &e)
                    {
                        cerr << "\nINVALID INPUT!!! Please enter an integer \n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    catch (const out_of_range &e)
                    {
                        cerr << "\n"
                             << e.what() << "\n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

                break;

            case 3:
                // edits number of enemies
                while (true)
                {
                    cout << "\nYou have selected enemies as your customisable option\n";
                    cout << "Please enter a new amount of enemies you want to have in your game (must be between 0 and 10)or enter the letter 'b' to go back: ";
                    cin >> new_amount;
                    if (new_amount == "b")
                    {
                        cout << "\nyou have selected to go back\n";
                        break;
                    }

                    if (stoi(new_amount) >= 0 && stoi(new_amount) <= 10)
                    {
                        total_enemies = stoi(new_amount);
                        cout << "\nThe amount of enemies you have chosen is " << total_enemies << "\n";
                        break;
                    }

                    // if user input invalid then inform user, clear cin to prevent erros and repeat options
                    else
                    {
                        cerr << "\nINVALID INPUT!!! Please enter one of the valid options \n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

                break;

            case 4:
                // edits the number of puzzles
                while (true)
                {
                    cout << "\nYou have selected puzzles as your customisable option\n";
                    cout << "Please enter a new amount of puzzles you want to have in your game (must be between 0 and 10)or enter the letter 'b' to go back: ";
                    cin >> new_amount;
                    if (new_amount == "b")
                    {
                        cout << "\nyou have selected to go back\n";
                        break;
                    }

                    if (stoi(new_amount) >= 0 && stoi(new_amount) <= 10)
                    {
                        total_puzzles = stoi(new_amount);
                        cout << "\nThe amount of puzzles you have chosen is " << total_puzzles << "\n";
                        break;
                    }

                    // if user input invalid then inform user, clear cin to prevent erros and repeat options
                    else
                    {
                        cerr << "\nINVALID INPUT!!! Please enter one of the valid options \n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

                break;

            case 5:
                // edits the number of swords availible to user
                while (true)
                {
                    cout << "\nYou have selected swords as your customisable option\n";
                    cout << "Please enter a new amount of swords you want to have per level (must be between 0 and 10)or enter the letter 'b' to go back: ";
                    cin >> new_amount;
                    if (new_amount == "b")
                    {
                        cout << "\nyou have selected to go back\n";
                        break;
                    }

                    if (stoi(new_amount) >= 0 && stoi(new_amount) <= 10)
                    {
                        total_swords = stoi(new_amount);
                        cout << "\nThe amount of swords you have chosen is " << total_swords << "\n";
                        break;
                    }

                    // if user input invalid then inform user, clear cin to prevent erros and repeat options
                    else
                    {
                        cerr << "\nINVALID INPUT!!! Please enter one of the valid options \n";
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }

            case 6:
                // returns to main menu screen
                return;

            default:
                // if user enters invalid option then inform user and repeat options
                cerr << "ERROR!!! You did not enter one of the 5 options" << endl;
                break;
            }
        }

        else
        {
            cerr << "INVALID INPUT!!! You can only enter integers \n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// this function places enemies around the maze
vector<Enemy> enemies;
void enemy_placement()
{
    int random_y, random_x;

    // loops through random coordinates until path is found
    while (true)
    {
        random_x = rand() % MAZE_WIDTH;
        random_y = rand() % MAZE_HEIGHT;

        if (maze[random_y][random_x] == '0')
        {
            break; // exits loop
        }
    }
    // creates enemy object and pushes it onto the enemy vector
    Enemy e = Enemy(random_y, random_x);
    e.enemy_active = true;
    enemies.push_back(e);
    maze[random_y][random_x] = 'E';
}

// this struct stores the game state once player pauses game
struct game_state
{
    int saved_player_x_position;
    int saved_player_y_position;
    int saved_score;
    int saved_current_level;
    int saved_game_end;
    int saved_active_sword;
    std::chrono::steady_clock::time_point game_loop_start_timepoint;
    vector<Enemy> saved_enemies;
};

// this saves the game state into a text file (saving_file.txt) file
void save_game_state(const game_state &state)
{
    ofstream write_to_file("saving_file.txt");
    write_to_file << state.saved_score << '\n';
    write_to_file << state.saved_current_level << '\n';
    write_to_file << state.saved_game_end << '\n';
    write_to_file << std::chrono::duration_cast<chrono::seconds>(std::chrono::steady_clock::now() - state.game_loop_start_timepoint).count() << '\n';

    for (int row = 0; row < MAZE_WIDTH; row++)
    {
        for (int column = 0; column < MAZE_HEIGHT; column++)
        {
            write_to_file << maze[row][column] << " ";
        }
        write_to_file << endl;
    }
    write_to_file.close();
}

// this reads the game state from the file and loads it into the game state structure variables
void load_game_state(game_state &state)
{
    ifstream read_from_file("saving_file.txt");
    read_from_file >> state.saved_score;
    read_from_file >> state.saved_current_level;
    read_from_file >> state.saved_game_end;
    int seconds;
    read_from_file >> seconds;
    state.game_loop_start_timepoint = std::chrono::steady_clock::now() - std::chrono::seconds(seconds);
    for (int row = 0; row < MAZE_WIDTH; row++)
    {
        for (int column = 0; column < MAZE_HEIGHT; column++)
        {
            read_from_file >> maze[row][column];
            if (maze[row][column] == '8')
            {
                state.saved_player_x_position = column;
                state.saved_player_y_position = row;
            }
        }
    }
    read_from_file.close();
}

// resets game state to default values in order to start a new game
void reset_game_state(game_state &state)
{
    state.saved_player_x_position = PLAYER_STARTING_X;
    state.saved_player_y_position = PLAYER_STARTING_Y;
    state.saved_score = 0;
    state.saved_current_level = 1;
    state.saved_game_end = false;
    state.saved_active_sword = false;
    state.game_loop_start_timepoint = std::chrono::steady_clock::now();
    state.saved_enemies.clear();
}

// resets game variables to default values in order to start a new game
void set_game_variables(game_state &state)
{
    player_y_position = state.saved_player_y_position;
    player_x_position = state.saved_player_x_position;
    score = state.saved_score;
    current_level = state.saved_current_level;
    game_end = state.saved_game_end;
    active_sword = state.saved_active_sword;
    game_paused = false;
}

void prepare_for_new_game()
{
    maze_template_creation(); // load grid size for maze
    // pushing starting positions onto stack and creating the player icon
    maze_path.push({PLAYER_STARTING_Y, PLAYER_STARTING_X});
    path_generation(PLAYER_STARTING_Y, PLAYER_STARTING_X);
    maze[PLAYER_STARTING_Y][PLAYER_STARTING_X] = PLAYER_ICON;

    puzzle_placement(); // loads new puzzles

    // places enemies around maze according to the total amount of enemies selected to be present in a round
    for (int element = 0; element < total_enemies; element++)
    {
        enemy_placement();
    }
}

void game_loop(game_state &state)
{
    // local variables
    string loading_text = "Loading next level";
    int available_swords = total_swords;

    // output onto cmd
    HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(handleOut, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(handleOut, consoleMode);

    // sets starting game variables
    set_game_variables(state);

    // sets time variables
    std::chrono::steady_clock::time_point sword_start, pause_start_time, pause_end_time, game_loop_timepoint;
    int total_paused_time = 0, game_play_duration = 0;

    // checks if game_loop_start_timepoint is 0 and sets it to the current time if it is
    if (state.game_loop_start_timepoint.time_since_epoch().count() == 0)
        state.game_loop_start_timepoint = std::chrono::steady_clock::now();

    // game_end to keep game running while game has not ended
    while (!game_end)
    {
        current_player_location(); // re-outputs maze

        // players moving mechanics input
        cout << "\nEnter move: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin >> player_mechanics;

        // switch statement allows the player to move using wasd
        switch ((int)player_mechanics)
        {
        case (int)'w':
        {
            maze[player_y_position][player_x_position] = '0';
            player_y_position--;
            // these sub conditions checks if the player walks into a wall and prevents them from doing so by reversing the change of movement
            if (maze[player_y_position][player_x_position] == '1')
            {
                player_y_position++;
                score--;
                maze[player_y_position][player_x_position] = '8';
            }

            else if (maze[player_y_position][player_x_position] == 'W')
            {
                game_end = true;
            }
            else if (maze[player_y_position][player_x_position] == 'P')
            {
                if (!puzzle_activated())
                {
                    maze[player_y_position][player_x_position] = '1';
                    player_y_position++;
                    maze[player_y_position][player_x_position] = '8';
                    score -= 10;
                }

                else
                {
                    maze[player_y_position][player_x_position] = '8';
                    score += 10;
                }
            }

            else
            {
                maze[player_y_position][player_x_position] = '8';
            }

            break;
        }
        case (int)'s':
        {
            maze[player_y_position][player_x_position] = '0';
            player_y_position++;
            if (maze[player_y_position][player_x_position] == '1')
            {
                player_y_position--;
                score--;
                maze[player_y_position][player_x_position] = '8';
            }

            else if (maze[player_y_position][player_x_position] == 'W')
            {
                game_end = true;
            }

            else if (maze[player_y_position][player_x_position] == 'P')
            {
                if (!puzzle_activated())
                {
                    maze[player_y_position][player_x_position] = '1';
                    player_y_position--;
                    maze[player_y_position][player_x_position] = '8';
                    score -= 10;
                }
                else
                {
                    maze[player_y_position][player_x_position] = '8';
                    score += 10;
                }
            }

            else
            {
                maze[player_y_position][player_x_position] = '8';
            }

            break;
        }
        case (int)'a':
        {
            maze[player_y_position][player_x_position] = '0';
            player_x_position--;
            if (maze[player_y_position][player_x_position] == '1')
            {
                player_x_position++;
                score--;
                maze[player_y_position][player_x_position] = '8';
            }

            else if (maze[player_y_position][player_x_position] == 'W')
            {
                game_end = true;
            }

            else if (maze[player_y_position][player_x_position] == 'P')
            {
                if (!puzzle_activated())
                {
                    maze[player_y_position][player_x_position] = '1';
                    player_x_position++;
                    maze[player_y_position][player_x_position] = '8';
                    score -= 10;
                }
                else
                {
                    maze[player_y_position][player_x_position] = '8';
                    score += 10;
                }
            }

            else
            {
                maze[player_y_position][player_x_position] = '8';
            }

            break;
        }
        case (int)'d':
        {
            maze[player_y_position][player_x_position] = '0';
            player_x_position++;
            if (maze[player_y_position][player_x_position] == '1')
            {
                player_x_position--;
                score--;
                maze[player_y_position][player_x_position] = '8';
            }

            else if (maze[player_y_position][player_x_position] == 'W')
            {
                game_end = true;
            }

            else if (maze[player_y_position][player_x_position] == 'P')
            {
                if (!puzzle_activated())
                {
                    maze[player_y_position][player_x_position] = '1';
                    player_x_position--;
                    maze[player_y_position][player_x_position] = '8';
                    maze[player_y_position - 1][player_x_position] = '1';
                    score -= 10;
                }
                else
                {
                    maze[player_y_position][player_x_position] = '8';
                    score += 10;
                }
            }

            else
            {
                maze[player_y_position][player_x_position] = '8';
            }

            break;
        }
        case (int)'/':
        {
            if (available_swords > 0)
            {
                active_sword = true;
                cout << "sword started";
                available_swords--;
                sword_start = std::chrono::steady_clock::now();
            }
            break;
        }
        case (int)'p':
        {
            if (active_sword)
            {
                break;
            }
            pause_start_time = std::chrono::steady_clock::now(); // Record the pause start time
            int option;

            cout << "\n";
            cout << "========\n";
            cout << " Paused \n";
            cout << "========\n";
            cout << "\n";
            cout << "1. Save and Quit to Main Menu\n";
            cout << "2. Resume\n";

            cin >> option;
            pause_end_time = std::chrono::steady_clock::now();          // Record the pause end time
            auto total_paused_time = pause_end_time - pause_start_time; // calculates the duration spent in the pause menu
            switch (option)
            {
            case 1:
                // saves game variables to game state structure
                state.saved_score = score;
                state.saved_current_level = current_level;
                state.saved_game_end = game_end;
                state.game_loop_start_timepoint += total_paused_time; // Add the total paused time from the start time so timer appears to pause
                save_game_state(state);
                return; // exits game loop and returns to main menu screen
                break;
            case 2:
                state.game_loop_start_timepoint += total_paused_time; // Add the total paused time from the start time so timer appears to pause
                game_paused = true;
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
        if (game_paused)
        {
            game_paused = false;
            continue;
        }

        cout << "\nCurrent Score: " << score << endl;

        for (auto &enemy : enemies)
        {
            if (enemy.enemy_active)
            {
                enemy.enemy_movement(player_y_position, player_x_position);
            }
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy &enemy)
                                     { return !enemy.enemy_active; }),
                      enemies.end());

        game_loop_timepoint = std::chrono::steady_clock::now();
        game_play_duration = std::chrono::duration_cast<std::chrono::seconds>(game_loop_timepoint - state.game_loop_start_timepoint).count();
        cout << "Timer: " << game_play_duration % 60 << "s" << endl;

        if (game_play_duration >= total_time)
        {
            game_end = true;
            lost = true;
        }

        auto sword_now = std::chrono::steady_clock::now();
        auto sword_duration = std::chrono::duration_cast<std::chrono::seconds>(sword_now - sword_start).count();

        if (active_sword && sword_duration >= 10)
        {
            active_sword = false;
            cout << "\ndeactivated sword\n";
        }

        if (game_end)
        {
            if (!lost)
            {
                cout << "CONGRATULATIONS!!!\n";

                cout << "You have passed level " << current_level << " out of " << total_levels << "\n";
                cout << "Your score is " << score << "\n";

                if (current_level != total_levels)
                {
                    current_level++;
                    available_swords = total_swords;
                    game_end = false;
                    maze_path = stack<pair<int, int>>();

                    maze[player_x_position][player_y_position] = '0';
                    player_x_position = PLAYER_STARTING_X;
                    player_y_position = PLAYER_STARTING_Y;

                    state.game_loop_start_timepoint = std::chrono::steady_clock::now();
                    prepare_for_new_game();

                    cout << "The next level will now load. Good luck\n";
                    cout << loading_text;
                    for (int i = 0; i < 3; i++)
                    {
                        this_thread::sleep_for(chrono::seconds(1));
                        cout << "." << flush;
                    }
                    this_thread::sleep_for(chrono::seconds(1));
                    cout << endl;
                }
                else
                {
                    cout << "\nWELL DONE!!! You have completed all the levels\n";
                    cout << "Your final score is " << score << "\n";
                    break;
                }
            }

            else
            {
                cout << "\nGAME OVER!!!!\n";
                cout << "\nYOU LOST\n";
                lost = false;
            }
        }
    };
}

int main()
{
    srand(time(NULL)); // to ensure maze generation is random at every time it loads
    int option;
    string loading_text = "Loading game";
    game_state state;

    // loops through case statements and checks for invalid input
    while (true)
    {
        // present user with text options
        cout << "\n";
        cout << "==========================\n";
        cout << "  Welcome to My Maze Game \n";
        cout << "==========================\n";
        cout << "\n";
        cout << "1. Start New Game\n";
        cout << "2. Continue Previous Game\n";
        cout << "3. Settings\n";
        cout << "4. Quit\n";
        cout << "enter option: ";
        cin >> option;

        if (!cin.fail())
        {
            switch (option)
            {
            case 1:
            {
                // deletes previous game state
                ofstream delete_file_contents("saving_file.txt", ios::out | ios::trunc);
                delete_file_contents.close();
                enemies.clear();
                state.saved_enemies.clear();
                reset_game_state(state);
                prepare_for_new_game();

                // loading text animation and then game starts
                cout << loading_text;
                for (int i = 0; i < 3; i++)
                {
                    this_thread::sleep_for(chrono::seconds(1));
                    cout << "." << flush;
                }
                this_thread::sleep_for(chrono::seconds(1));
                cout << "\nGame loaded successfully!\n";
                game_loop(state);
                break;
            }

            case 2:
            {
                // checks if there is a previous game to load
                ifstream file_content("saving_file.txt");
                if (file_content.peek() == ifstream::traits_type::eof())
                {
                    cout << "No previous game to load\n";
                }
                else
                {
                    cout << "content in file\n";
                    load_game_state(state);
                    set_game_variables(state);
                    game_loop(state);
                }
                break;
            }

            case 3:
            {
                // deletes previous game state
                ofstream delete_file_contents("saving_file.txt", ios::out | ios::trunc);
                delete_file_contents.close();

                settings();// settings page
                break;
            }
                

            case 4:
                // teminates game
                exit(0);
                break;

            default:
                // invalid input check and repeat options
                cerr << "ERROR!!! You did not enter one of the 3 options" << endl;
            }
        }

        // if user enters wrong datatype then inform user and clear cin to prevent game from crashing and repeat options
        else
        {
            cerr << "INVALID INPUT!!! You can only enter integers \n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    return 0;
}