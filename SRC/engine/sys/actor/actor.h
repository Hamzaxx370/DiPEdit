// ============================================================================
// File: Actor.h
// 
// Purpose: Defines the actor system classes and the game's actor list enum, with
// cact_base being the base actor, cact_manager being the actor manager, and e_actid
// being the actor list mapping
// 
// ============================================================================

#ifndef __ACTION_H_
#define __ACTION_H_

#include "defs.h"

// Forward declare the actor list
enum class e_actid : int;

// Bit flags for exec_flag
enum class e_act_exec : int {
	none = 0,
	done = 1 << 0,
	pause = 1 << 1,
};

// Base actor class
class cact_base {
public:
	e_actid m_act_id; // ActTbl index

	// Linking
	cact_base* m_parent;
	cact_base* m_child;
	cact_base* m_sibling;

	float act_speed; // For animation stuff

	/**
	* @brief Sets a flag in the actor.
	* @param The flag to set.
	*/
	void set_exec_flag ( e_act_exec flag ) {
		m_exec_flag |= ( int ) flag;
	};

	cact_base ( cact_base* p_parent, e_actid id );
	cact_base ( ) { };
	virtual ~cact_base ( );
private:
	int m_exec_flag; // Bitfield
	cact_base* m_delete_next;

	/**
	* @brief First execution function for the actor, used for 
	* interacting with other actors.
	*/
	virtual void exec0 ( );

	/**
	* @brief Second execution function for the actor, used for
	* reacting to changes and finalizing state
	*/
	virtual void exec1 ( );

	/**
	* @brief Adds a child actor to this actor
	* @param p_act Pointer to the child actor to add
	*/
	void add_child ( cact_base* p_act );

	friend class cact_manager;
};

// The actor manager
class cact_manager {
public:
	cact_base* m_root_act;

	/**
	* @brief Initializes the actor tree
	* @param act_num Number of actors to allocate in the actor table
	*/
	void init ( e_actid act_num );

	/**
	* @brief The update function for the actor manager, executes all actors and
	* gathers finished ones for deletion
	*/
	int idle ( );

	/**
	* @brief Registers an actor in the actor table
	* @param p_act Pointer to the actor to register
	*/
	void regist_actor ( cact_base* p_act );

	/**
	* @brief Gets actor from the actor tree by id
	* @param actid the actor id
	*/
	cact_base* get_actor ( e_actid actid );

	/**
	* @brief Gets a free actor id in the specified range
	* @param st Start of the range
	* @param en End of the range
	*/
	e_actid get_free_id ( e_actid st, e_actid en );

	cact_manager ( );
	~cact_manager ( );
private:
	cact_base* m_delete_list; // Linked list of actors to delete

	cact_base** m_act_tbl; // For ease of access, prevents lots of pointer usage
	int m_act_tbl_num;


	/**
	* @brief Recursive function for the first execution pass for actors
	* @param p_act Pointer to the actor to execute
	*/
	void act_exec0 ( cact_base* p_act );

	/**
	* @brief Recursive function for the second execution pass for actors, executing
	and gathering finished actors for deletion
	* @param p_act Pointer to the actor to execute and check for deletion
	*/
	void act_exec1 ( cact_base* p_act );

	/**
	* @brief Adds an actor to this frame's deletion list
	* @param p_act Pointer to the actor to add
	*/
	void add_delete ( cact_base* p_act );


	/**
	* @brief Delete an actor and relink the other ones
	* @param p_act Pointer to the actor to delete
	*/
	void delete_actor ( cact_base* p_act );
};

#endif