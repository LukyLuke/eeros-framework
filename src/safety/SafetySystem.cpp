#include <eeros/safety/SafetySystem.hpp>
#include <eeros/core/Fault.hpp>

namespace eeros {
  namespace safety {

    uint8_t SafetySystem::instCount = 0;
    SafetySystem* SafetySystem::instance = nullptr;

    SafetySystem::SafetySystem(SafetyProperties& safetyProperties, double period)
        : log(logger::Logger::getLogger('S')),
          currentLevel(nullptr),
          privateContext(this),
          period(period) {
      if (++instCount > 1) { // only one instance is allowed
        throw Fault("only one instance of the safety system is allowed");
      }
      if (!setProperties(safetyProperties)) {
        throw Fault("verification of safety properties failed!");
      }
      instance = this;
    }

    SafetySystem::~SafetySystem() {
      instCount--;
    }

    SafetyLevel& SafetySystem::getCurrentLevel(void) {
      if (currentLevel) {
        return *currentLevel;
      } else {
        // TODO define error number and send error message to logger
        throw Fault("currentLevel not defined");
      }
    }

    bool SafetySystem::setProperties(SafetyProperties& safetyProperties) {
      if (safetyProperties.verify()) {
        properties = safetyProperties;
        currentLevel = properties.getEntryLevel();
        currentLevel->nofActivations = 0;
        nextLevel = currentLevel;
        log.warn() << "safety system verified: " << (int)properties.levels.size() << " safety levels are present";
        return true;
      }
      return false;
    }

    void SafetySystem::triggerEvent(SafetyEvent event, SafetyContext* context) {
      if(currentLevel) {
        SafetyLevel* newLevel = currentLevel->getDestLevelForEvent(event, context == &privateContext);
        if(newLevel != nullptr) {
          // prioritize multiple events,
          // can be called by different threads
          mtx.lock();
          if(nextLevel == currentLevel) {
            nextLevel = newLevel;
            nextLevel->nofActivations = 0;
          } else if(newLevel->id < nextLevel->id) {
            nextLevel = newLevel;
            nextLevel->nofActivations = 0;
          }
          mtx.unlock();
          log.info() << "triggering event \'" << event << "\' in level '" << currentLevel << "\': transition to safety level: '" << nextLevel << "\'";
        } else {
          log.error() << "triggering event \'" << event << "\' in level '" << currentLevel << "\': no transition for this event";
        }
      } else {
        // TODO define error number and send error message to logger
        throw Fault("current level not defined");
      }
    }

    const SafetyProperties* SafetySystem::getProperties() const {
      return &properties;
    }

    double SafetySystem::getPeriod() const {
      return period;
    }

    void SafetySystem::run() {
      // level must only change before safety system runs or after run method has finished
      if (nextLevel != nullptr) {
        currentLevel = nextLevel;
      }

      if (currentLevel != nullptr) {
        // 1) Get currentLevel
        SafetyLevel* level = currentLevel;
        level->nofActivations++;

        // 2) Read inputs
        for (auto ia : level->inputAction) {
          if (ia != nullptr) {
            SafetyLevel* oldLevel = currentLevel;
            if (ia->check(&privateContext)) {
              SafetyLevel* newLevel = nextLevel;
              using namespace logger;
              hal::InputInterface* input = (hal::InputInterface*)(ia->getInput());
              if(oldLevel != newLevel) {
                log.info() << "level changed due to input action: " << input->getId()
                           << " from level '" << oldLevel << "'"
                           << " to level '" << newLevel << "'";
              }
            }
          }
        }

        // 3) Execute level action
        if (level->action != nullptr) {
          level->action(&privateContext);
        }

        // 4) Set outputs
        for (auto oa : level->outputAction) {
          if (oa != nullptr) {
            oa->set();
          }
        }
        if (nextLevel != nullptr) {
          currentLevel = nextLevel;
        }
      } else {
        log.error() << "current level is null!";
      }
    }

    void SafetySystem::exitHandler() {
      SafetySystem* ss = SafetySystem::instance;
      if (ss) {
        ss->properties.exitFunction(&ss->privateContext);
      }
    }
  };
};
