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

#include "nmmagent.h"
#include <memory>
#include <chrono>
#include <omp.h>

using namespace std;

NMMAgent::NMMAgent(void)
	: eval(new Evaluator()), phase(Positioning), i_am_white(true), last_from("0 0"), last_to("0 0")
{}

NMMAgent::~NMMAgent(void){}

void NMMAgent::write_play(SmrtState state)
{
    if(state->mill)
    {
        out.open("mill.txt", ios::out);
        out << gPos2Coord[state->removed];
        out.close();
    }

    // grava a jogada
    out.open("move.txt", ios::out);
    switch(phase)
    {
    case Positioning:
        out << gPos2Coord[state->moved];
        break;
    case Playing:
        out << gPos2Coord[state->moved] << " " << gPos2Coord[state->moved_to];
        break;
    }
    out.close();
}

bool NMMAgent::repeated_play(SmrtState state)
{
    if(phase == Positioning)    return false;

    return (gPos2Coord[state->moved_to] == last_from) && gPos2Coord[state->moved] == last_to;
}

void NMMAgent::set_evaluator_weights(int positioning[6], int playing[7], int flying[4])
{
	eval->set_evaluation_weight(positioning, playing, flying);
}

int NMMAgent::evaluate(Board &board, const bool &white_turn, const bool &mill)
{
	if(phase == Positioning)
		return eval->evaluate_position(board, white_turn, mill);
	else if((/*white_turn &&*/ board.get_num_white_pieces() == 3) ||
			(/* !white_turn &&*/ board.get_num_black_pieces() == 3))
		return eval->evaluate_flying(board, white_turn, mill);
	else
		return eval->evaluate_playing(board, white_turn, mill);
}

void NMMAgent::play(const SmrtState &root)
{
    int black_pcs = root->board->get_num_black_pieces();
    int white_pcs = root->board->get_num_white_pieces();
	// dynamic deepning
	int max = 7;                // default max level tree
	if(phase == Positioning)    // if is posititioning phase
        max = 5;
    else if(black_pcs == 3 && white_pcs == 3) // if both players are on flying mode
        max = 4;
	else if(black_pcs == 3 || white_pcs == 3) // if only one player is on flying mode
        max = 5;
	write_play(minimax_decision(root, max));
}

SmrtState NMMAgent::decide(const SmrtState &root)
{
    int black_pcs = root->board->get_num_black_pieces();
    int white_pcs = root->board->get_num_white_pieces();
	// dynamic deepning
	int max = 7;
	if(phase == Positioning)
        max = 5;
    else if(black_pcs == 3 && white_pcs == 3)
        max = 4;
	else if(black_pcs == 3 || white_pcs == 3)
        max = 5;
	write_play(minimax_decision(root, max));

	SmrtState deicision = minimax_decision(root, max);
	if(phase == Positioning)
		deicision->terminal = false;

	return deicision;
}

std::vector<SmrtState> NMMAgent::expand(const SmrtState &root, const bool &white_turn)
{
	return sucessor(root->board, white_turn);
}

std::vector<SmrtState> NMMAgent::sucessor(const SmrtBoard &board, const bool &white_turn)
{
	if(phase == Positioning)
		return sucessor_positioning(board, white_turn);
	else if((white_turn && board->get_num_white_pieces() < 4) ||
			(!white_turn && board->get_num_black_pieces() < 4))
		return sucessor_playing_flying(board, white_turn);
	else
		return sucessor_playing(board, white_turn);
}

std::vector<SmrtState> NMMAgent::sucessor_positioning(const SmrtBoard &board, const bool &white_turn)
{
	std::vector<SmrtState> suc;
	for(int i = 0; i < BOARD_SPOT; i++)
	{
		bool mill = false;
		if(board->is_placeable(i, white_turn, mill))
		{
			// build a new board if this piece
			shared_ptr<Board> c = board->copy();
			c->place_spot(i, white_turn);

			// in case of mill, build new boards simulating removed pieces
			if(mill)
			{
				// list of boards with removed pieces
				std::vector<SmrtState> l = sucessor_removing(c, white_turn, Place, i);
				suc.insert(suc.begin() + suc.size(), l.begin(), l.end());
			}
			else
			{
				int utility = evaluate(*c, white_turn, false) - !evaluate(*c, !white_turn, false);
				SmrtState state = SmrtState(new State(c, i, Place, utility, false, false, false));
				// add board sorted by its score
				suc.insert(suc.begin() + binarysrc(state->utility, 0, suc.size(), suc), state);
			}
		}
	}
	return suc;
}

std::vector<SmrtState> NMMAgent::sucessor_playing(const SmrtBoard &board, const bool &white_turn)
{
	std::vector<SmrtState> suc;
	for(int i = 0; i < BOARD_SPOT; i++)
	{
		// Not expand empty spot or adversary piece
		Spot* s = board->get_spot(i);
		if(s->status == SS_Empty ||
			(s->status == SS_White && !white_turn) ||
			(s->status == SS_Black && white_turn))
			continue;

		bool mill = false, finish = false, win = false;
		for(int act = 1; act < 5; act++)
		{
			NMMAction action = (NMMAction)act;
			// verify if movement in any direction is valid
			if(board->is_movable(i, s->status, action, mill, finish, win))
			{
				// create a board 
				shared_ptr<Board> c = board->copy();
				int to = c->move_spot(i, action);
				// verify if adversary is blocked
				finish |= c->is_opp_blocked(white_turn? SS_Black : SS_White);

				// in case of mill, build new boards simulating removed pieces
				if(mill)
				{
					// list of boards with removed pieces
					std::vector<SmrtState> l = sucessor_removing(c, white_turn, action, i, to);
					suc.insert(suc.begin() + suc.size(), l.begin(), l.end());
				}
				else
				{
					int utility = evaluate(*c, white_turn, false) - !evaluate(*c, !white_turn, false);
					SmrtState state = SmrtState(new State(c, i, action, utility, finish, win, false));
					state->moved_to = to;
					// add board sorted by its score
					suc.insert(suc.begin() + binarysrc(state->utility, 0, suc.size(), suc), state);
				}
			}
		}
	}
	return suc;
}

std::vector<SmrtState> NMMAgent::sucessor_playing_flying(const SmrtBoard &board, const bool &white_turn)
{
	std::vector<SmrtState> suc;
	for(int f = 0; f < BOARD_SPOT; f++)
	{
		Spot* from = board->get_spot(f);
		// Not expand empty spot or adversary piece
		if(from->status == SS_Empty ||
		  (from->status == SS_White && !white_turn) ||
		  (from->status == SS_Black && white_turn))
			continue;

		for(int t = 0; t < BOARD_SPOT; t++)
		{
			// if the same position, continue
			if(f == t)
				continue;

			bool mill = false, finish = false, win = false;
			// Check if spot is valid 
			if(board->is_flyable(f, t, from->status, mill, finish, win))
			{
				shared_ptr<Board> c = board->copy();
				c->fly_spot(f, t);
				// verify if adversary is blocked
				finish |= c->is_opp_blocked(white_turn? SS_Black : SS_White);

				// in case of mill, build new boards simulating removed pieces
				if(mill)
				{
					// list of boards with removed pieces
					std::vector<SmrtState> l = sucessor_removing(c, white_turn, Fly, f, t);
					suc.insert(suc.begin() + suc.size(), l.begin(), l.end());
				}
				else
				{
					int utility = evaluate(*c, white_turn, false) - !evaluate(*c, !white_turn, false);
					SmrtState state = SmrtState(new State(c, f, Fly, utility, finish, win, false));
					state->moved_to = t;
					// add board sorted by its score
					suc.insert(suc.begin() + binarysrc(state->utility, 0, suc.size(), suc), state);
				}
			}
		}
	}
	return suc;
}

std::vector<SmrtState> NMMAgent::sucessor_removing(const SmrtBoard &board, const bool &white_turn, NMMAction a, int piece, int to, bool force)
{
	SpotStatus my_sts = white_turn ? SS_White : SS_Black;

	std::vector<SmrtState> suc;
	for(int i = 0; i < BOARD_SPOT; i++)
	{
		Spot* s = board->get_spot(i);
		// Not expand empty spot or adversary piece
		if(s->status == SS_Empty ||
		  (s->status == SS_White && white_turn) ||
		  (s->status == SS_Black && !white_turn))
			continue;

		bool finish = false, win = false;
		if(board->is_removable(i, my_sts, finish, win, force))
		{
			// if game phase is placement, it is not possible to win
			if(phase == Positioning)
				finish = win = false;

			shared_ptr<Board> c = board->copy();
			c->remove_spot(i);
			// verify if adversary is blocked
			finish |= c->is_opp_blocked(white_turn? SS_Black : SS_White);

			int utility = evaluate(*c, white_turn, true) - evaluate(*c, !white_turn, false);
			SmrtState state = SmrtState(new State(c, piece, a, utility, finish, win, true, i));

			state->moved_to = to;

			suc.insert(suc.begin() + binarysrc(state->utility, 0, suc.size(), suc), state);
		}
	}
	// if no new boards it means every piece is within a mill, call it again with force activated.
	if(suc.size() == 0)
		return sucessor_removing(board, white_turn, a, piece, 0, true);
	return suc;
}

int NMMAgent::binarysrc(int &utility, int b, int e,  std::vector<SmrtState> &l)
{
	if(e == b)	return e;

	int m = (e+b)/2;
	if(utility > l[m]->utility)
		return binarysrc(utility, b, m, l);
	else
		return binarysrc(utility, m+1, e, l);
}

SmrtState NMMAgent::minimax_decision(const SmrtState &root, int max_level)
{
	SmrtState bestMove;
	std::cout << "minimax started, max level is " << max_level << "... ";
	std::cout.flush();
	max_value_pab(root, max_level, bestMove, -INT_MAX, INT_MAX, true);
    std::cout << "minimax run completely. ";
    std::cout.flush();
	return bestMove;
}

int NMMAgent::max_value_pab(const SmrtState &state, int tree_level, SmrtState &move, int alpha, int beta, bool first_call)
{
	if(state->terminal) return  -state->utility;
	if(tree_level < 1) return -state->utility;
	int v = -INT_MAX;

	std::vector<SmrtState> suc = sucessor(state->board, i_am_white);
	int size = suc.size();

    // im time based game, it garuantee a play movement
    if(first_call)
    {
        // if the best play is not repeated, it's ok
        if(!repeated_play(suc[0])) write_play(suc[0]);
        // otherwise return second best
        else  write_play(suc[1]);
    }
	
	for(int i = 0; i < size; ++i)
	{
		SmrtState opp_move;
		int m = min_value_pab(suc[i], tree_level-1, opp_move, alpha, beta);
		//Max (v, Min)
		if(v < m)
		{
            // If it's not Max first level, play normally
            if(!first_call){  v = m;	move = suc[i]; }
            // Otherwise just chooses the play if not repeated or we are losing the game
            else if(!repeated_play(suc[i]) || m < 0){  v = m;	move = suc[i]; }
        }

		if(v >= beta)
		{
			for(int j = 0; j < size; j++)	std::get_deleter<SmrtState>(suc[j]);
			break;
		}
		//alpha max(alpha, v)
		if(v > alpha) { alpha = v; }

		std::get_deleter<SmrtState>(suc[i]);
	}
	return v;
}

int NMMAgent::min_value_pab(const SmrtState &state, int tree_level, SmrtState &move, int alpha, int beta)
{
	if(state->terminal) return  state->utility;
	if(tree_level < 1) return state->utility;
	int v = INT_MAX;

	std::vector<SmrtState> suc = sucessor(state->board, !i_am_white);
	int size = suc.size();
	
	for(int i = 0; i < size; ++i)
	{
        int m = max_value_pab(suc[i], tree_level-1, move, alpha, beta, false);
		//Min (v, Max)
		if(v > m){  v = m;	move = suc[i]; }
		// if v <= alpha
		if(v <= alpha)
		{
			for(int j = 0; j < size; j++)	std::get_deleter<SmrtState>(suc[j]);
			break;
		}
		if(v < beta) { beta = v; }
		std::get_deleter<SmrtState>(suc[i]);
	}
	return v;
}

int NMMAgent::max_value(const SmrtState &state, int max_tree_level, SmrtState &move)
{

	if(state->terminal) return  -state->utility;
	if(max_tree_level < 1) return -state->utility;
	int v = -INT_MAX;

	std::vector<SmrtState> suc = sucessor(state->board, i_am_white);
	int size = suc.size();
	for(int i = 0; i < size; ++i)
	{
		SmrtState opp_move;
		int m = min_value(suc[i], max_tree_level-1, opp_move);
		if(v < m)
		{
			v = m;
			move = suc[i];
		}
		std::get_deleter<SmrtState>(suc[i]);
	}
	return v;
}

int NMMAgent::min_value(const SmrtState &state, int max_tree_level, SmrtState &move)
{
	if(state->terminal) return  state->utility;
	if(max_tree_level < 1) return state->utility;
	int v = INT_MAX;

	std::vector<SmrtState> suc = sucessor(state->board, !i_am_white);
	int size = suc.size();
	for(int i = 0; i < size; ++i)
	{
		int m = max_value(suc[i], max_tree_level-1, move);
		if(v > m)
		{
			v = m;
			move = suc[i];
		}
		std::get_deleter<SmrtState>(suc[i]);
	}
	return v;
}
