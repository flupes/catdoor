#ifndef VISUAL_FEEDBACK_H
#define VISUAL_FEEDBACK_H

// Just an interface class for convinience
class VisualFeedback {
 public:
  virtual void error() = 0;
  virtual void warning() = 0;
  virtual void ok() = 0;
  virtual void alive() = 0;
};

#endif
