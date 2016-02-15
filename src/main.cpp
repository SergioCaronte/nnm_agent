/*
* Copyright (C) 2015 Sergio Nunes da Silva Junior 
*
* C++ Nine Men's Morris Agent using alpha-beta prunning algorithm
* Assignment of Artificial Intelligence Course - 2/2015
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* author: Sergio Nunes da Silva Junior
* contact: sergio.nunes@dcc.ufmg.com.br
* Universidade Federal de Minas Gerais (UFMG) - Brazil
*/

#include <iostream>
#include <stdio.h>
#include <chrono>

#include "nmmagent.h"
#include "board.h"
#include <cstring>

bool finish( SmrtState root, bool &white_win, bool white_turn)
{
	if(root->terminal)
	{
		white_win = root->white_win;
		return true;
	}
	return false;
}

SmrtState human_player(NMMAgent *useless, SmrtState curr, bool i_am_white, GamePhase phase, int unused)
{
	// command variables
	int piece, to, removed = -1;	char action;
	// current board
	SmrtBoard b = curr->board;
	// history play variables 
	bool mill = false, finish = false, win = false;
	// action
	NMMAction acted = Place;
	SpotStatus ss = i_am_white ? SS_White : SS_Black;

	// check whether it is flying phase for human player
	bool flying = (i_am_white && curr->board->get_num_white_pieces() == 3) ||
					(!i_am_white && curr->board->get_num_black_pieces() == 3);

	// flag for valid play
	bool valid = false;
	while(!valid)
	{
		if(phase == Positioning)
		{
			cout << "choose position to place a piece: ";
			cin >> piece;
			action = 'p';
		}
		else if(!flying)
		{
			cout << "choose your piece to move: ";
			cin >> piece;
			cout << "type action(u,d,l,r):";
			cin >> action;
			cout << "\n";
		}
		else
		{
			cout << "choose your piece to fly: ";
			cin >> piece;
			action = 'f';
			cout << "type destination: ";
			cin >> to;
		}

		if(action == 'p')
		{
			// check whether it is valid
			if(b->is_placeable(piece, i_am_white, mill))
			{
				valid = true;
				b->place_spot(piece, i_am_white);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}
		}
		else if(action == 'u' && phase == Playing)
		{
			acted = Mv_Up;
			if(b->is_movable(piece, ss, acted, mill, finish, win))
			{
				valid = true;
				b->move_spot(piece, acted);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}

		}
		else if(action == 'd' && phase == Playing)
		{
			acted = Mv_Down;
			if(b->is_movable(piece, ss, acted, mill, finish, win))
			{
				valid = true;
				b->move_spot(piece, acted);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}

		}
		else if(action == 'r' && phase == Playing)
		{
			acted = Mv_Right;
			if(b->is_movable(piece, ss, acted, mill, finish, win))
			{
				valid = true;
				b->move_spot(piece, acted);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}
		}
		else if(action == 'l' && phase == Playing)
		{
			acted = Mv_Left;
			if(b->is_movable(piece, ss, acted, mill, finish, win))
			{
				valid = true;
				b->move_spot(piece, acted);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}
		}
		else if(action == 'f')
		{
			acted = Place;
			if(b->is_flyable(piece, to, ss, mill, finish, win))
			{
				valid = true;
				b->fly_spot(piece, to);
				if(mill)
				{
					bool can_remove = false;
					while(!can_remove)
					{
						cout << "mill, choose a piece to remove: ";
						cin >> removed;
						can_remove = b->is_removable(removed, ss, finish, win);
						// check whether remotion is valid
						if(can_remove)	b->remove_spot(removed);
					}
				}
			}
			else // if it is not placeable, therefore is not valid
			{
				valid = false;
				std::cout << "invalid play\n";
			}
		}
	}
	curr = SmrtState(new State(b, piece, acted, 0, finish, i_am_white, mill, removed));
	if(phase == Playing)
	{
        finish |= b->is_opp_blocked(i_am_white ? SS_Black : SS_White);
		curr->terminal = finish;
    }
	return curr;
}

SmrtState ai_player(NMMAgent *a, SmrtState curr, bool unused, GamePhase phase, int random_level)
{
	if(random_level > 1)
	{
		std::vector<SmrtState> suc = a->expand(curr, a->get_color_pieces());
		curr = suc[rand()%random_level%suc.size()];
	}
	else
	{
		curr = a->decide(curr);
	}
	return curr;
}

inline SmrtBoard load_board(std::ifstream &file)
{
    SmrtBoard brd = SmrtBoard(new Board());
    char row[7];

    // board row 1
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[0] != 'o') brd->place_spot(A0, row[0] == 'W');
    if(row[3] != 'o') brd->place_spot(D0, row[3] == 'W');
    if(row[6] != 'o') brd->place_spot(G0, row[6] == 'W');

    // board row 2
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[1] != 'o') brd->place_spot(B1, row[1] == 'W');
    if(row[3] != 'o') brd->place_spot(D1, row[3] == 'W');
    if(row[5] != 'o') brd->place_spot(F1, row[5] == 'W');

    // board row 3
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[2] != 'o') brd->place_spot(C2, row[2] == 'W');
    if(row[3] != 'o') brd->place_spot(D2, row[3] == 'W');
    if(row[4] != 'o') brd->place_spot(E2, row[4] == 'W');

    // board row 4
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[0] != 'o') brd->place_spot(A3, row[0] == 'W');
    if(row[1] != 'o') brd->place_spot(B3, row[1] == 'W');
    if(row[2] != 'o') brd->place_spot(C3, row[2] == 'W');
    if(row[4] != 'o') brd->place_spot(E3, row[4] == 'W');
    if(row[5] != 'o') brd->place_spot(F3, row[5] == 'W');
    if(row[6] != 'o') brd->place_spot(G3, row[6] == 'W');

    // board row 5
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[2] != 'o') brd->place_spot(C4, row[2] == 'W');
    if(row[3] != 'o') brd->place_spot(D4, row[3] == 'W');
    if(row[4] != 'o') brd->place_spot(E4, row[4] == 'W');

    // board row 6
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[1] != 'o') brd->place_spot(B5, row[1] == 'W');
    if(row[3] != 'o') brd->place_spot(D5, row[3] == 'W');
    if(row[5] != 'o') brd->place_spot(F5, row[5] == 'W');

    // board row 7
    file.getline(row, 10);
    //printf("%s\n", row);
    if(row[0] != 'o') brd->place_spot(A6, row[0] == 'W');
    if(row[3] != 'o') brd->place_spot(D6, row[3] == 'W');
    if(row[6] != 'o') brd->place_spot(G6, row[6] == 'W');

    return brd;
}

void play(const char* filename)
{
    std::ifstream file(filename);
	// file of game state
    if(!file.is_open())
        printf("Failed to open input file (%s) \n", filename);

    NMMAgent* a = new NMMAgent();
//    int aphase1[6] = {28, 26, 1, 6, 12, 7};
//    int aphase2[7] = {34, 43, 10, 8, 7, 42, 1086};
//    int aphase3[4] = {10, 1, 16, 1190};
// Agent evaluation function weights
    int aphase1[6] = {18, 26, 1, 6, 12, 7};
    int aphase2[7] = {14, 43, 10, 8, 7, 42, 1086};
    int aphase3[4] = {10, 1, 16, 1190};
    a->set_evaluator_weights(aphase1, aphase2, aphase3);

    std::string color, type;
    file >> color;
    file >> type;
    if(type == "placement")
    {
        std::string qtpc;
        file >> qtpc;
    }
    // if it was mill, we just rename the mill file to move
    // as the mill choice was recored in a instante before
    else if(type == "mill")
    {
        rename("mill.txt", "move.txt");
        std::cout << "mill play ";
        std::cout.flush();
        return;
    }
    // parsing game state file
    char dump[255]; file.getline(dump, 255);
    // check the last 3 plays
    for(int i = 0; i < 3; i++)
    {
        char play[255]; file.getline(play, 255);
        // if it is my play and it is [mo]vement
        if(play[0] == color.c_str()[0] && (play[6] == 'm' && play[7] == 'o'))
        {
            std::string playstr = string(play);
            std::string from = playstr.substr(15, 3);
            std::string to = playstr.substr(19, 3);
            //printf("from:'%s' to:'%s'\n", from.c_str(), to.c_str());
            a->set_last_play(from, to);
        }
    }
    //printf("color: %s\ntype: %s\nplays:\n%s\n%s\n%s\n", color.c_str(), type.c_str(), play3, play2, play1);

    SmrtBoard brd = load_board(file);
    //std::cout << brd << "\n\n";
    SmrtState state = SmrtState(new State(brd, 0, Place, 0, false, false));

    a->set_color_pieces(color == "white");
    a->set_game_phase(type == "placement"? Positioning : Playing);
    a->play(state);

}

int main(int argc, char* argv[])
{
	// parsing type of game
	if(strcmp(argv[1], "-bot") == 0)
	{
        std::cout << "\n";
        std::cout.flush();
        auto start = chrono::system_clock::now();
        play(argv[2]);
        auto end = chrono::system_clock::now();
		auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);
        long double elapsed = duration.count()/1000000.0;
        std::cout << "elapsed time: " << elapsed << "ms";
        std::cout.flush();

	}
	else if(strcmp(argv[1], "-human") == 0)
	{
		srand(time_t(NULL));
		int turn = 0;

		//agente 1
		NMMAgent* a = new NMMAgent();
		int aphase1[6] = {18, 26, 1, 6, 12, 7};
		int aphase2[7] = {14, 43, 10, 8, 7, 42, 1086};
		int aphase3[4] = {10, 1, 16, 1190};
		a->set_evaluator_weights(aphase1, aphase2, aphase3);
		a->set_color_pieces(true);

		SmrtState root = SmrtState(new State(SmrtBoard(new Board()), 0, Place, 0, false, false));
		bool white_win = false;
		bool white_turn = true;

		char piece;
		std::cout << "do you want to play as white? (y/n):\n";
		std::cin >> piece;

		auto player1 = ai_player;
		auto player2 = human_player;
		if(piece == 'y')
		{
			player1 = human_player;
			player2 = ai_player;
			a->set_color_pieces(false);
		}

		while(!finish(root, white_win, true))
		{
			if(white_turn)
				root = player1(a, root, true, a->get_game_phase(), 1);
			else
				root = player2(a, root, false, a->get_game_phase(), 1);
			++turn;
			std::cout << "TURN: " << turn << (white_turn? " W\t" : " B\t") << root << "\n";
			std::cout << root->board << "\n\n";
			if(turn == 18)
			{
				std::cout << "Positioning Over - Playing started\n";
				a->set_game_phase(Playing);
			}
			white_turn = !white_turn;
		}
		if(white_win)
			std::cout << "White won!\n";
		else
			std::cout << "Black won!\n";
	}
	else if(strcmp(argv[1], "-botxbot") == 0)
	{
		srand(time_t(NULL));
		int turn = 0;

		//agent 1
		NMMAgent* a = new NMMAgent();
		int aphase1[6] = {18, 26, 1, 6, 12, 7};
		int aphase2[7] = {14, 43, 10, 8, 7, 47, 1086};
		int aphase3[4] = {10, 1, 16, 1190};
		a->set_evaluator_weights(aphase1, aphase2, aphase3);
		a->set_color_pieces(true);

		//agent 2
		NMMAgent* b = new NMMAgent();
		int bphase1[6] = {18, 26, 1, 6, 21, 7};
		int bphase2[7] = {42, 28, 16, 8, 24, 19, 949};
		int bphase3[4] = {23, 18, 5, 1096};
		b->set_evaluator_weights(bphase1, bphase2, bphase3);
		b->set_color_pieces(false);

		SmrtState root = SmrtState(new State(SmrtBoard(new Board()), 0, Place, 0, false, false));
		bool white_win = false;
		bool white_turn = true;
		auto player1 = ai_player;
		auto player2 = ai_player;

		while(!finish(root, white_win, true))
		{
			if(white_turn)
				root = player1(a, root, white_turn, a->get_game_phase(), 1);
			else
				root = player2(b, root, white_turn, a->get_game_phase(), 1);

			++turn;
			if(turn > 85) break;
			std::cout << "TURNO: " << turn << (white_turn? " W\t" : " B\t") << root << "\n";
			std::cout << root->board << "\n\n";
			if(turn == 18)
			{
				std::cout << "Positioning Over - Playing started\n";
				a->set_game_phase(Playing);
				b->set_game_phase(Playing);
			}
			white_turn = !white_turn;
		}

		if(white_win)
		{
			std::cout << "White won!\n";
		}
		else if(turn > 85)
		{
			std::cout << "Draw!\n";
		}
		else
		{
			std::cout << "Black won!\n";
		}
	}

	return 0;
}
