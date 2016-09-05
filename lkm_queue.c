#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

#include "signal.h"
#include "defs.h"

#define SIGNAL_QUEUE_SIZE 100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Armin Widegreen <armin.widegreen@gmail.com>");
MODULE_DESCRIPTION("A LKM (misc) char-device queue");


//------------------------------------------------------------------------------

struct queue_node {
  struct signal_t* data;
  struct queue_node* next;
};

//------------------------------------------------------------------------------

struct signal_queue {
  wait_queue_head_t read_queue;
  struct mutex lock;

  struct queue_node* front;
  struct queue_node* rear;

  unsigned long size;
  unsigned long max_size;
};

//------------------------------------------------------------------------------

struct signal_queue* global_queue;

//------------------------------------------------------------------------------

void
q_dequeue(struct signal_queue* queue) {
  struct queue_node* temp = queue->front;
  if (queue->front == NULL)
  { // queue is empty
    return;
  }

  if (queue->front == queue->rear)
  {
    queue->front = queue->rear = NULL;
  }
  else
  {
    queue->front = queue->front->next;
  }

  --queue->size;
  /*printk(KERN_INFO "Signal denqueued, new size: %lu!\n", queue->size);*/
  kfree(temp->data);
  kfree(temp);
}

//------------------------------------------------------------------------------

struct signal_t*
q_front(struct signal_queue* queue)
{
  if (queue->front == NULL)
  { // queue is empty
    return NULL;
  }

  return queue->front->data;
}

//------------------------------------------------------------------------------

void
q_clear(struct signal_queue* queue)
{
  while (q_front(queue))
    q_dequeue(queue);
}

//------------------------------------------------------------------------------

void
q_enqueue(struct signal_queue* queue, struct signal_t* sig)
{
  struct queue_node* temp = NULL;
  if (queue->size >= queue->max_size)
    return;

  temp = kzalloc(sizeof(*queue->front), GFP_KERNEL);
  if (unlikely(!temp))
    goto out_free;
  temp->data = sig;
  temp->next = NULL;

  ++queue->size;

  if (queue->front == NULL && queue->rear == NULL)
  { // empty queue
    queue->front = queue->rear = temp;
    goto out;
  }

  queue->rear->next = temp;
  queue->rear = temp;
out:
  /*printk(KERN_INFO "Signal enqueued, new size: %lu!\n", queue->size);*/
  return;
out_free:
  printk(KERN_INFO "Something went wrong when enqueueing!\n");
  q_clear(queue);
  kfree(queue);

}

//------------------------------------------------------------------------------

bool
q_empty(struct signal_queue* queue)
{
  return (queue->front == NULL && queue->rear == NULL);
}

//------------------------------------------------------------------------------

static struct signal_queue* queue_alloc(unsigned long size)
{
  struct signal_queue *queue = NULL;

  queue = kzalloc(sizeof(*queue), GFP_KERNEL);
  if (unlikely(!queue))
    goto out;
  queue->front = NULL;
  queue->rear = NULL;
  queue->size = 0;
  queue->max_size = SIGNAL_QUEUE_SIZE;

  init_waitqueue_head(&queue->read_queue);

  mutex_init(&queue->lock);

out:
  return queue;
}

//------------------------------------------------------------------------------

static void queue_free(struct signal_queue *queue)
{
  q_clear(queue);
  kfree(queue);
}

//------------------------------------------------------------------------------

static int queue_dev_open(struct inode *inode, struct file *file)
{
  return 0;
}

//------------------------------------------------------------------------------

static ssize_t queue_dev_read(struct file *file, char __user * out,
    size_t size, loff_t * off)
{
  struct signal_queue *queue = global_queue;
  struct signal_t* signal;
  ssize_t result;

  if (mutex_lock_interruptible(&queue->lock)) {
    result = -ERESTARTSYS;
    goto out;
  }

  while (q_empty(queue)) {
    mutex_unlock(&queue->lock);
    if (file->f_flags & O_NONBLOCK) {
      result = -EAGAIN;
      goto out;
    }
    // TODO
    if (wait_event_interruptible(queue->read_queue, !q_empty(queue))) {
      result = -ERESTARTSYS;
      goto out;
    }
    if (mutex_lock_interruptible(&queue->lock)) {
      result = -ERESTARTSYS;
      goto out;
    }
  }

  signal = q_front(queue);
  size = sizeof(*signal);
  if (copy_to_user(out, signal, size)) {
    result = -EFAULT;
    goto out_unlock;
  }
  q_dequeue(queue);

  result = size;

out_unlock:
  mutex_unlock(&queue->lock);
out:
  return result;
}

//------------------------------------------------------------------------------

static ssize_t queue_dev_write(struct file *file, const char __user * in,
    size_t size, loff_t * off)
{
  struct signal_queue* queue = global_queue;
  ssize_t result;
  struct signal_t* signal = NULL;

  if (mutex_lock_interruptible(&queue->lock)) {
    result = -ERESTARTSYS;
    goto out;
  }

  signal = kzalloc(sizeof(*signal), GFP_KERNEL);
  if (unlikely(!signal)) {
     result = -EFAULT;
     goto out_unlock;
  }

  if (copy_from_user(signal, in, size)) {
    result = -EFAULT;
    goto out_unlock;
  }
  q_enqueue(queue, signal);

  wake_up_interruptible(&queue->read_queue);

  result = size;
out_unlock:
  mutex_unlock(&queue->lock);
out:
  return result;
}

//------------------------------------------------------------------------------

static int queue_dev_close(struct inode *inode, struct file *file)
{
  return 0;
}

//------------------------------------------------------------------------------

static struct file_operations queue_dev_fops = {
  .owner = THIS_MODULE,
  .open = queue_dev_open,
  .read = queue_dev_read,
  .write = queue_dev_write,
  .release = queue_dev_close,
  .llseek = noop_llseek
};

//------------------------------------------------------------------------------

static struct miscdevice queue_dev_misc_device = {
  .minor = MISC_DYNAMIC_MINOR,
  .name = DEVICE_NAME,
  .fops = &queue_dev_fops
};

//------------------------------------------------------------------------------

static int __init queue_dev_init(void)
{
  global_queue = queue_alloc(SIGNAL_QUEUE_SIZE);
  if (unlikely(!global_queue)) {
    return -ENOMEM;
  }

  misc_register(&queue_dev_misc_device);
  printk(KERN_INFO "queue_dev device has been registered, with queue size %d\n",
      SIGNAL_QUEUE_SIZE);

  return 0;
}

//------------------------------------------------------------------------------

static void __exit queue_dev_exit(void)
{
  queue_free(global_queue);

  misc_deregister(&queue_dev_misc_device);
  printk(KERN_INFO "queue_dev device has been unregistered\n");
}

//------------------------------------------------------------------------------

module_init(queue_dev_init);
module_exit(queue_dev_exit);
