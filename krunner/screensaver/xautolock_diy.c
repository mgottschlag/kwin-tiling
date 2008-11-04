/*****************************************************************************
 *
 * Authors: Michel Eyckmans (MCE) & Stefan De Troch (SDT)
 *
 * Content: This file is part of version 2.x of xautolock. It implements
 *          the stuff used when the program is not using a screen saver
 *          extension and thus has to use the good old "do it yourself"
 *          approach for detecting user activity.
 *
 *          The basic idea is that we initially traverse the window tree,
 *          selecting SubstructureNotify on all windows and adding each
 *          window to a temporary list. About +- 30 seconds later, we 
 *          scan this list, now asking for KeyPress events. The delay
 *          is needed in order to interfere as little as possible with
 *          the event propagation mechanism. Whenever a new window is 
 *          created by an application, a similar process takes place. 
 *
 *          Please send bug reports etc. to eyckmans@imec.be.
 * 
 * --------------------------------------------------------------------------
 * 
 * Copyright 1990,1992-1999,2001-2002 by Stefan De Troch and Michel Eyckmans.
 * 
 * Versions 2.0 and above of xautolock are available under version 2 of the
 * GNU GPL. Earlier versions are available under other conditions. For more
 * information, see the License file.
 *
 *****************************************************************************/

#include <X11/Xlib.h>
#include <stdlib.h>
#include <time.h>

#include "xautolock_c.h"

static void selectEvents (Window window, Bool substructureOnly);

/*
 *  Window queue management.
 */
typedef struct item
{
  Window       window;
  time_t       creationtime;
  struct item* next;
} xautolock_anItem, *xautolock_item;

static struct 
{
  Display*     display;
  struct item* head;
  struct item* tail;
} queue;

static void
addToQueue (Window window)
{
  xautolock_item newItem = malloc(sizeof(xautolock_anItem));

  newItem->window = window;
  newItem->creationtime = time (0);
  newItem->next = 0;

  if (!queue.head) queue.head = newItem;
  if ( queue.tail) queue.tail->next = newItem;

  queue.tail = newItem;
}

static void
processQueue (time_t age)
{
  if (queue.head)
  {
    time_t now = time (0);
    xautolock_item current = queue.head;

    while (current && current->creationtime + age < now)
    {
      selectEvents (current->window, False);
      queue.head = current->next;
      free (current);
      current = queue.head;
    }

    if (!queue.head) queue.tail = 0;
  }
}

/*
 *  Function for selecting all interesting events on a given 
 *  (tree of) window(s).
 */
static void 
selectEvents (Window window, Bool substructureOnly)
{
  Window            root;              /* root window of the window */
  Window            parent;            /* parent of the window      */
  Window*           children;          /* children of the window    */
  unsigned          nofChildren = 0;   /* number of children        */
  unsigned          i;                 /* loop counter              */
  XWindowAttributes attribs;           /* attributes of the window  */

  if( xautolock_ignoreWindow( window ))
      return;
 /*
  *  Start by querying the server about the root and parent windows.
  */
  if (!XQueryTree (queue.display, window, &root, &parent,
                   &children, &nofChildren))
  {
    return;
  }

  if (nofChildren) (void) XFree ((char*) children);

 /*
  *  Build the appropriate event mask. The basic idea is that we don't
  *  want to interfere with the normal event propagation mechanism if
  *  we don't have to.
  *
  *  On the root window, we need to ask for both substructureNotify 
  *  and KeyPress events. On all other windows, we always need 
  *  substructureNotify, but only need Keypress if some other client
  *  also asked for them, or if they are not being propagated up the
  *  window tree.
  */
#if 0
  if (substructureOnly)
  {
    (void) XSelectInput (queue.display, window, SubstructureNotifyMask);
  }
  else
  {
    if (parent == None) /* the *real* rootwindow */
    {
      attribs.all_event_masks = 
      attribs.do_not_propagate_mask = KeyPressMask;
    }
    else if (!XGetWindowAttributes (queue.display, window, &attribs))
#else
    {
    if (!XGetWindowAttributes (queue.display, window, &attribs))
#endif
    {
      return;
    }

#if 0
    (void) XSelectInput (queue.display, window, 
                           SubstructureNotifyMask
                         | (  (  attribs.all_event_masks
                               | attribs.do_not_propagate_mask)
                            & KeyPressMask));
#else
    {
    int mask = SubstructureNotifyMask | attribs.your_event_mask;
    if( !substructureOnly )
        {
        mask |=            (  (  attribs.all_event_masks
                               | attribs.do_not_propagate_mask)
                            & KeyPressMask  );
        }
    (void) XSelectInput (queue.display, window, mask );
    }
#endif

  }

 /*
  *  Now ask for the list of children again, since it might have changed
  *  in between the last time and us selecting SubstructureNotifyMask.
  *
  *  There is a (very small) chance that we might process a subtree twice:
  *  child windows that have been created after our XSelectinput() has
  *  been processed but before we get to the XQueryTree() bit will be 
  *  in this situation. This is harmless. It could be avoided by using
  *  XGrabServer(), but that'd be an impolite thing to do, and since it
  *  isn't required...
  */
  if (!XQueryTree (queue.display, window, &root, &parent,
                   &children, &nofChildren))
  {
    return;
  }

 /*
  *  Now do the same thing for all children.
  */
  for (i = 0; i < nofChildren; ++i)
  {
    selectEvents (children[i], substructureOnly);
  }

  if (nofChildren) (void) XFree ((char*) children);
}

#if 0
/*
 *  Function for processing any events that have come in since 
 *  last time. It is crucial that this function does not block
 *  in case nothing interesting happened.
 */
void
processEvents (void)
{
  while (XPending (queue.display))
  {
    XEvent event;

    if (XCheckMaskEvent (queue.display, SubstructureNotifyMask, &event))
    {
      if (event.type == CreateNotify)
      {
        addToQueue (event.xcreatewindow.window);
      }
    }
    else
    {
      (void) XNextEvent (queue.display, &event);
    }

   /*
    *  Reset the triggers if and only if the event is a
    *  KeyPress event *and* was not generated by XSendEvent().
    */
    if (   event.type == KeyPress
        && !event.xany.send_event)
    {
      resetTriggers ();
    }
  }

 /*
  *  Check the window queue for entries that are older than
  *  CREATION_DELAY seconds.
  */
  processQueue ((time_t) CREATION_DELAY);
}
#else
void xautolock_processEvent( XEvent* event )
{
      if (event->type == CreateNotify)
      {
        addToQueue (event->xcreatewindow.window);
      }
   /*
    *  Reset the triggers if and only if the event is a
    *  KeyPress event *and* was not generated by XSendEvent().
    */
    if (   event->type == KeyPress
        && !event->xany.send_event)
    {
      xautolock_resetTriggers ();
    }
}

void xautolock_processQueue()
{
 /*
  *  Check the window queue for entries that are older than
  *  CREATION_DELAY seconds.
  */
  processQueue ((time_t) CREATION_DELAY);
}
#endif


/*
 *  Function for initialising the whole shebang.
 */
void
xautolock_initDiy (Display* d)
{
  int s;

  queue.display = d;
  queue.tail = 0;
  queue.head = 0; 

  for (s = -1; ++s < ScreenCount (d); )
  {
    Window root = RootWindowOfScreen (ScreenOfDisplay (d, s));
    addToQueue (root);
#if 0
    selectEvents (root, True);
#endif
  }
}
