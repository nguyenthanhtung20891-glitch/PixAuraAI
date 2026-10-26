#include "intelligence_resources.hpp"
namespace pixaura::intelligence {
Owner& Owner::shared(){static Owner owner;return owner;}
}
