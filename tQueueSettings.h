//
// You received this file as part of Finroc
// A Framework for intelligent robot control
//
// Copyright (C) Finroc GbR (finroc.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//----------------------------------------------------------------------
/*!\file    plugins/data_ports/tQueueSettings.h
 *
 * \author  Max Reichardt
 *
 * \date    2013-05-08
 *
 * \brief   Contains tQueueSettings
 *
 * \b tQueueSettings
 *
 * Contains all relevant settings for input port queues.
 * Can be passed to port constructors in order to create ports with input queues.
 *
 */
//----------------------------------------------------------------------
#ifndef __plugins__data_ports__tQueueSettings_h__
#define __plugins__data_ports__tQueueSettings_h__

//----------------------------------------------------------------------
// External includes (system with <>, local with "")
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Internal includes with ""
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Namespace declaration
//----------------------------------------------------------------------
namespace finroc
{
namespace data_ports
{

//----------------------------------------------------------------------
// Forward declarations / typedefs / enums
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Class declaration
//----------------------------------------------------------------------
//! Queue Settings
/*!
 * Contains all relevant settings for input port queues.
 * Can be passed to port constructors in order to create ports with input queues.
 */
class tQueueSettings
{

//----------------------------------------------------------------------
// Public methods and typedefs
//----------------------------------------------------------------------
public:

  /*!
   * \param dequeue_all_queue Should queue support dequeueing all elements at once?
   *                          (or rather dequeueing elements one after the other)
   *                          (All at once is more efficient)
   * \param maximum_queue_length Maximum number of elements in queue.
   *                             A value of -1 indicates that the queue has (virtually) no size limit.
   *                             This is somewhat dangerous: If elements in a queue of unlimited size are
   *                             not fetched, this causes continuous memory allocation for new buffers.
   */
  explicit tQueueSettings(bool dequeue_all_queue, int maximum_queue_length = -1) :
    dequeue_all_queue(dequeue_all_queue),
    maximum_queue_length(maximum_queue_length)
  {}

  /*!
   * \return Should queue support dequeueing all elements at once?
   */
  bool DequeueAllQueue() const
  {
    return dequeue_all_queue;
  }

  /*!
   * \return Maximum number of elements in queue
   */
  int GetMaximumQueueLength() const
  {
    return maximum_queue_length;
  }

//----------------------------------------------------------------------
// Private fields and methods
//----------------------------------------------------------------------
private:

  /*!
   * Should queue support dequeueing all elements at once?
   * (or rather dequeueing elements one after the other)
   * (All at once is more efficient)
   */
  bool dequeue_all_queue;

  /*!
   * Maximum number of elements in queue.
   * A value of -1 indicates that queue has (virtually) no size limit
   */
  int maximum_queue_length;

};

//----------------------------------------------------------------------
// End of namespace declaration
//----------------------------------------------------------------------
}
}


#endif
