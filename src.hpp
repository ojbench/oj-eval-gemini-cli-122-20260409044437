#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  return NotifyLateEvent::GetNotification(n) + generator_(n);
}

class Memo {
 public:
  Memo() = delete;

  Memo(int duration) : duration_(duration), current_time_(0) {
    heads_ = new EventNode*[duration_ + 1];
    tails_ = new EventNode*[duration_ + 1];
    for (int i = 0; i <= duration_; ++i) {
      heads_[i] = nullptr;
      tails_[i] = nullptr;
    }
  }

  ~Memo() {
    for (int i = 0; i <= duration_; ++i) {
      EventNode* curr = heads_[i];
      while (curr != nullptr) {
        EventNode* next = curr->next;
        delete curr;
        curr = next;
      }
    }
    delete[] heads_;
    delete[] tails_;
  }

  void AddEvent(const Event *event) {
    if (const NotifyBeforeEvent* nbe = dynamic_cast<const NotifyBeforeEvent*>(event)) {
      int time = nbe->GetNotifyTime();
      if (time <= duration_) {
        AddNode(time, new EventNode(event, 0));
      }
    } else {
      int time = event->GetDeadline();
      if (time <= duration_) {
        AddNode(time, new EventNode(event, 0));
      }
    }
  }

  void Tick() {
    current_time_++;
    if (current_time_ > duration_) return;

    EventNode* curr = heads_[current_time_];
    heads_[current_time_] = nullptr;
    tails_[current_time_] = nullptr;

    while (curr != nullptr) {
      EventNode* next_node = curr->next;
      curr->next = nullptr;

      if (!curr->event->IsComplete()) {
        std::cout << curr->event->GetNotification(curr->notify_count) << std::endl;

        if (const NotifyBeforeEvent* nbe = dynamic_cast<const NotifyBeforeEvent*>(curr->event)) {
          if (curr->notify_count == 0) {
            int deadline = nbe->GetDeadline();
            curr->notify_count = 1;
            if (deadline <= duration_) {
              AddNode(deadline, curr);
              curr = nullptr;
            }
          }
        } else if (const NotifyLateEvent* nle = dynamic_cast<const NotifyLateEvent*>(curr->event)) {
          int next_time = current_time_ + nle->GetFrequency();
          curr->notify_count += 1;
          if (next_time <= duration_) {
            AddNode(next_time, curr);
            curr = nullptr;
          }
        }
      }

      if (curr != nullptr) {
        delete curr;
      }
      curr = next_node;
    }
  }

 private:
  struct EventNode {
    const Event* event;
    int notify_count;
    EventNode* next;
    EventNode(const Event* e, int count) : event(e), notify_count(count), next(nullptr) {}
  };

  int duration_;
  int current_time_;
  EventNode** heads_;
  EventNode** tails_;

  void AddNode(int time, EventNode* node) {
    if (heads_[time] == nullptr) {
      heads_[time] = node;
      tails_[time] = node;
    } else {
      tails_[time]->next = node;
      tails_[time] = node;
    }
  }
};

#endif
