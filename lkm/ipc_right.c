#include "mach_includes.h"
#include "ipc_right.h"
#include "ipc_port.h"
#include "debug.h"
#include <linux/slab.h>

struct mach_port_right* ipc_right_new(darling_mach_port_t* port, mach_port_right_t type)
{
	struct mach_port_right* right;
	
	right = (struct mach_port_right*) kmalloc(sizeof(struct mach_port_right), GFP_KERNEL);
	if (right == NULL)
		return NULL;
	
	right->port = port;
	right->type = type;
	right->num_refs = 1;
	
	switch (type)
	{
		case MACH_PORT_RIGHT_RECEIVE:
			break;
		case MACH_PORT_RIGHT_SEND:
			port->num_srights++;
			break;
		case MACH_PORT_RIGHT_SEND_ONCE:
			port->num_sorights++;
			break;
	}
	
	list_add(&right->reflist, &port->refs);
	
	return right;
}

void ipc_right_put(struct mach_port_right* right)
{
	darling_mach_port_t* port;
	
	if (right == NULL)
		return;
	
	port = right->port;
	
	if (right->type == MACH_PORT_RIGHT_RECEIVE)
	{
		ipc_port_put(port);
	}
	else if (right->type == MACH_PORT_RIGHT_SEND
		|| right->type == MACH_PORT_RIGHT_SEND_ONCE)
	{
		if (PORT_IS_VALID(port))
		{
			if (right->type == MACH_PORT_RIGHT_SEND)
				port->num_srights--;
			else if (right->type == MACH_PORT_RIGHT_SEND_ONCE)
				port->num_sorights--;
		}
	}
	list_del(&right->reflist);
	
	kfree(right);
	
	debug_msg("Deallocated right %p\n", right);
}

kern_return_t ipc_right_mod_refs(struct mach_port_right* right,
		mach_port_right_t type,
		int refchange)
{
	if (right->type != type)
		return KERN_INVALID_RIGHT;
	
	if (right->num_refs + refchange < 0)
		return KERN_INVALID_VALUE;
	
	if (refchange > 0 && (right->type == MACH_PORT_RIGHT_RECEIVE 
			|| right->type == MACH_PORT_RIGHT_SEND_ONCE || right->type == MACH_PORT_RIGHT_PORT_SET))
	{
		debug_msg("ipc_right_mod_refs() on right %p -> KERN_UREFS_OVERFLOW\n", right);
		return KERN_UREFS_OVERFLOW;
	}
	
	// We deviate from documentation in having only one right type per right name
	right->num_refs += refchange;
	debug_msg("ipc_right_mod_refs() on right=%p, refchange=%d, num_refs=%d\n",
		right, refchange, right->num_refs);
	
	return KERN_SUCCESS;
}
