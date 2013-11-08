#ifndef __USERINTERFACE_H__
#define __USERINTERFACE_H__

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "cinder/Thread.h"
#include "cinder/gl/Texture.h"
#include "Environment.h"
#include "Utilities.h"
#include "Resources.h"
#include "Sculpt.h"
#include <boost/function.hpp>
#include <vector>
using namespace ci;
using namespace ci::gl;

class LeapInteraction;
class ThreeFormApp;

class Menu {
public:

  // the values for a particular group should be consecutive
  enum MenuEntryType {
    // quick access menu
    STRENGTH = 0,
    SIZE,
    TYPE,
    COLOR,

    // tools
    TOOL_PAINT,
    TOOL_PUSH,
    TOOL_SWEEP,
    TOOL_FLATTEN,
    TOOL_SMOOTH,
    TOOL_SHRINK,
    TOOL_GROW,

    // size
    SIZE_AUTO,

    // strength
    STRENGTH_LOW,
    STRENGTH_MEDIUM,
    STRENGTH_HIGH,

    // material
    MATERIAL_PORCELAIN,
    MATERIAL_PLASTIC,
    MATERIAL_GLASS,
    MATERIAL_STEEL,
    MATERIAL_CLAY,

    // auto spin
    SPIN_OFF,
    SPIN_SLOW,
    SPIN_MEDIUM,
    SPIN_FAST,

    // wireframe
    WIREFRAME_OFF,
    WIREFRAME_ON,

    // symmetry
    SYMMETRY_OFF,
    SYMMETRY_ON,

    // history
    HISTORY_UNDO,
    HISTORY_REDO,

    // time of day
    TIME_DAWN,
    TIME_NOON,

    // environment
    ENVIRONMENT_ISLANDS,
    ENVIRONMENT_MOONSCAPE,
    ENVIRONMENT_RIVER,
    ENVIRONMENT_DESERT,
    ENVIRONMENT_REDWOOD,
    ENVIRONMENT_JUNGLE_CLIFF,
    ENVIRONMENT_JUNGLE,
    ENVIRONMENT_ARCTIC,

    // main
    MAIN_ABOUT,
    MAIN_TUTORIAL,
    MAIN_EXIT,

    // file
    FILE_LOAD,
    FILE_SAVE_OBJ,
    FILE_SAVE_PLY,
    FILE_SAVE_STL,
    FILE_RESET,

    // sound
    SOUND_ON,
    SOUND_OFF,

    NUM_ICONS
  };

  Menu();
  void update(const std::vector<Vec4f>& tips, Sculpt* sculpt);
  void setNumEntries(int num);
  void draw() const;
  int checkCollision(const Vector2& pos) const;
  bool checkPadCollision(const Vector2& pos) const;
  void toRadialCoordinates(const Vector2& pos, float& radius, float& angle) const;

  bool isActivated() const { return m_activation.value >= 1.0f; }
  bool hasSelectedEntry() const { return m_curSelectedEntry >= 0; }
  static void setWindowSize(const ci::Vec2i& size);

//private:

  struct MenuEntry {
    enum DrawMethod { ICON, COLOR, STRING, CIRCLE };
    MenuEntry() : m_position(Vector2::Zero()), m_radius(0.02f), m_value(0.0f) {
      m_hoverStrength.value = 0.0f;
      m_activationStrength.value = 0.0f;
    }
    void draw(const Menu* parent, bool selected) const {
      static const float ICON_ACTIVATION_BONUS_SCALE = 0.1f;
      static const float SHAPE_ACTIVATION_BONUS_SCALE = 0.5f;
      const float opacity = std::max(m_activationStrength.value, parent->m_activation.value);
      const float brightness = selected ? 1.0f : 0.5f + 0.5f*std::max(m_activationStrength.value, m_hoverStrength.value);
      const ci::Vec2f pos(m_position.x(), m_position.y());
      ci::ColorA color(brightness, brightness, brightness, opacity);
      gl::color(color);
      if (drawMethod == ICON) {
        ci::gl::Texture& tex = Menu::m_icons[m_entryType];
        const ci::Vec2i size = tex.getSize();
        glPushMatrix();
        glTranslatef(pos.x, pos.y, 0.0f);
        const float scale = (ICON_ACTIVATION_BONUS_SCALE*m_activationStrength.value) + parent->relativeToAbsolute(0.09f) / size.x;
        glScalef(scale, scale, scale);
        glTranslatef(-size.x/2.0f, -size.y/2.0f, 0.0f);
        if (opacity > 0.01f) {
          glColor4f(brightness, brightness, brightness, opacity);
          gl::draw(tex);
        }
        glPopMatrix();
      } else if (drawMethod == CIRCLE) {
        const float scale = 1.0f + (SHAPE_ACTIVATION_BONUS_SCALE*m_activationStrength.value);
        gl::drawSolidCircle(pos, parent->relativeToAbsolute(scale * m_radius), 40);
      } else if (drawMethod == STRING) {
        gl::drawStringCentered(toString(), pos - Vec2f(0, Menu::FONT_SIZE/2.0f), color, Menu::m_font);
      }
    }
    std::string toString() const {
      if (drawMethod == ICON || drawMethod == STRING) {
        switch(m_entryType) {
        case Menu::STRENGTH: return "Strength"; break;
        case Menu::SIZE: return "Size"; break;
        case Menu::TYPE: return "Tool"; break;
        case Menu::COLOR: return "Color"; break;
        case Menu::TOOL_PUSH: return "Press"; break;
        case Menu::TOOL_SWEEP: return "Smear"; break;
        case Menu::TOOL_FLATTEN: return "Flatten"; break;
        case Menu::TOOL_SMOOTH: return "Smooth"; break;
        case Menu::TOOL_SHRINK: return "Repel"; break;
        case Menu::TOOL_GROW: return "Inflate"; break;
        case Menu::TOOL_PAINT: return "Paint"; break;
        case Menu::SIZE_AUTO: return "Auto"; break;
        case Menu::STRENGTH_LOW: return "Low"; break;
        case Menu::STRENGTH_MEDIUM: return "Medium"; break;
        case Menu::STRENGTH_HIGH: return "High"; break;
        case Menu::MATERIAL_PLASTIC: return "Plastic"; break;
        case Menu::MATERIAL_PORCELAIN: return "Porcelain"; break;
        case Menu::MATERIAL_GLASS: return "Glass"; break;
        case Menu::MATERIAL_STEEL: return "Steel"; break;
        case Menu::MATERIAL_CLAY: return "Clay"; break;
        case Menu::SPIN_OFF: return "Off"; break;
        case Menu::SPIN_SLOW: return "Slow"; break;
        case Menu::SPIN_MEDIUM: return "Medium"; break;
        case Menu::SPIN_FAST: return "Fast"; break;
        case Menu::WIREFRAME_ON: return "On"; break;
        case Menu::WIREFRAME_OFF: return "Off"; break;
        case Menu::SYMMETRY_ON: return "On"; break;
        case Menu::SYMMETRY_OFF: return "Off"; break;
        case Menu::HISTORY_REDO: return "Redo"; break;
        case Menu::HISTORY_UNDO: return "Undo"; break;
        case Menu::TIME_DAWN: return "Dawn"; break;
        case Menu::TIME_NOON: return "Noon"; break;
        case Menu::ENVIRONMENT_ISLANDS: return "Islands"; break;
        case Menu::ENVIRONMENT_MOONSCAPE: return "Moonscape"; break;
        case Menu::ENVIRONMENT_RIVER: return "River"; break;
        case Menu::ENVIRONMENT_DESERT: return "Desert"; break;
        case Menu::ENVIRONMENT_REDWOOD: return "Redwood"; break;
        case Menu::ENVIRONMENT_JUNGLE_CLIFF: return "Jungle-Cliff"; break;
        case Menu::ENVIRONMENT_JUNGLE: return "Jungle"; break;
        case Menu::ENVIRONMENT_ARCTIC: return "Arctic"; break;
        case Menu::MAIN_ABOUT: return "About"; break;
        case Menu::MAIN_TUTORIAL: return "Tutorial"; break;
        case Menu::MAIN_EXIT: return "Exit"; break;
        case Menu::FILE_LOAD: return "Load"; break;
        case Menu::FILE_SAVE_OBJ: return "Export OBJ"; break;
        case Menu::FILE_SAVE_PLY: return "Export PLY"; break;
        case Menu::FILE_SAVE_STL: return "Export STL"; break;
        case Menu::FILE_RESET: return "Reload"; break;
        case Menu::SOUND_ON: return "On"; break;
        case Menu::SOUND_OFF: return "Off"; break;
        }
        return "";
      } else {
        std::stringstream ss;
        ss << m_value;
        return ss.str();
      }
    }
    Sculpt::SculptMode toSculptMode() const {
      switch (m_entryType) {
      case Menu::TOOL_GROW: return Sculpt::INFLATE; break;
      case Menu::TOOL_SHRINK: return Sculpt::DEFLATE; break;
      case Menu::TOOL_SMOOTH: return Sculpt::SMOOTH; break;
      case Menu::TOOL_FLATTEN: return Sculpt::FLATTEN; break;
      case Menu::TOOL_SWEEP: return Sculpt::SWEEP; break;
      case Menu::TOOL_PUSH: return Sculpt::PUSH; break;
      case Menu::TOOL_PAINT: return Sculpt::PAINT; break;
      }
      return Sculpt::INVALID;
    }

    Material toMaterial() const {
      Material mat;
      switch (m_entryType) {
      case Menu::MATERIAL_PLASTIC:
        mat.reflectionFactor = 0.1f;
        mat.surfaceColor << 0.0f, 0.4f, 1.0f;
        mat.reflectionBias = 0.5f;
        break;
      case Menu::MATERIAL_PORCELAIN:
        mat.reflectionFactor = 0.1f;
        mat.surfaceColor << 1.0f, 0.95f, 0.9f;
        mat.reflectionBias = 0.5f;
        break;
      case Menu::MATERIAL_GLASS:
        mat.diffuseFactor = 0.15f;
        mat.reflectionFactor = 0.7f;
        mat.surfaceColor << 0.4f, 0.45f, 0.5f;
        mat.refractionIndex = 0.45f;
        break;
      case Menu::MATERIAL_STEEL:
        mat.reflectionFactor = 0.5f;
        mat.surfaceColor << 0.2f, 0.25f, 0.275f;
        break;
      case Menu::MATERIAL_CLAY:
        mat.surfaceColor << 0.7f, 0.6f, 0.3f;
        break;
      }
      return mat;
    }
    float toSpinVelocity() const {
      switch (m_entryType) {
      case Menu::SPIN_OFF: return 0.0f; break;
      case Menu::SPIN_SLOW: return 0.63f; break;
      case Menu::SPIN_MEDIUM: return 2.13f; break;
      case Menu::SPIN_FAST: return 5.37f; break;
      }
      return 0.0f;
    }
    bool toWireframe() const {
      switch (m_entryType) {
      case Menu::WIREFRAME_ON: return true; break;
      case Menu::WIREFRAME_OFF: return false; break;
      }
      return false;
    }
    bool toSymmetry() const {
      switch (m_entryType) {
      case Menu::SYMMETRY_ON: return true; break;
      case Menu::SYMMETRY_OFF: return false; break;
      }
      return false;
    }
    Environment::TimeOfDay toTimeOfDay() const {
      switch (m_entryType) {
      case Menu::TIME_DAWN: return Environment::TIME_DAWN; break;
      case Menu::TIME_NOON: return Environment::TIME_NOON; break;
      }
      return Environment::TIME_DAWN;
    }
    std::string toEnvironmentName() const {
      switch (m_entryType) {
      case Menu::ENVIRONMENT_ISLANDS: return "Islands"; break;
      case Menu::ENVIRONMENT_MOONSCAPE: return "Moonscape"; break;
      case Menu::ENVIRONMENT_RIVER: return "River"; break;
      case Menu::ENVIRONMENT_DESERT: return "Desert"; break;
      case Menu::ENVIRONMENT_REDWOOD: return "Redwood"; break;
      case Menu::ENVIRONMENT_JUNGLE_CLIFF: return "Jungle-Cliff"; break;
      case Menu::ENVIRONMENT_JUNGLE: return "Jungle"; break;
      case Menu::ENVIRONMENT_ARCTIC: return "Arctic"; break;
      }
      return "";
    }

    MenuEntryType m_entryType;
    DrawMethod drawMethod;
    float m_radius;
    float m_value;
    Vector2 m_position;
    float m_angleStart;
    float m_angleWidth;
    float m_wedgeStart;
    float m_wedgeEnd;
    ci::Color m_color;
    Utilities::ExponentialFilter<float> m_hoverStrength;
    Utilities::ExponentialFilter<float> m_activationStrength;
  };

  float getSweepStart() const { return static_cast<float>(-m_sweepAngle/2.0f + m_angleOffset); }
  float getSweepEnd() const { return getSweepStart() + m_sweepAngle; }
  const MenuEntry& getSelectedEntry() const { return m_entries[m_curSelectedEntry]; }
  static float getOpacity(float activation) {
    static const float NORMAL_OPACITY = 0.4f;
    static const float ACTIVATED_OPACITY = 1.0f;
    return NORMAL_OPACITY + (ACTIVATED_OPACITY-NORMAL_OPACITY)*activation;
  }
  float getRingRadius() const { return 0.75f * m_outerRadius; }
  ci::Vec2f relativeToAbsolute(const Vector2& pos) const {
    Vector2 scaled = pos;
    //scaled.x() = (scaled.x() - 0.5f)/m_windowAspect + 0.5f;
    //scaled.y() = (scaled.y() - 0.5f)*m_windowAspect + 0.5f;
    scaled = scaled.cwiseProduct(m_windowSize);
    return ci::Vec2f(scaled.data());
  }
  float relativeToAbsolute(float radius) const {
    //return m_windowDiagonal * radius;
    return radius * m_windowSize.y();
  }
  float absoluteToRelative(float radius) const {
    return radius / m_windowSize.y();
  }

  static const float FONT_SIZE;
  static const float RING_THICKNESS_RATIO;
  static const float STRENGTH_UI_MULT;
  static const float BASE_OUTER_RADIUS;
  static const float BASE_INNER_RADIUS;
  static const float OUTER_RADIUS_PER_ENTRY;
  static const float SWEEP_ANGLE;

  Utilities::ExponentialFilter<float> m_activation;
  float m_outerRadius;
  float m_innerRadius;
  float m_angleOffset;
  float m_sweepAngle;
  std::vector<MenuEntry> m_entries;
  int m_curSelectedEntry;
  int m_prevSelectedEntry;
  double m_selectionTime;
  double m_deselectTime;
  Vector2 m_position;
  std::string m_name;
  static ci::Font m_font;
  static ci::Font m_boldFont;
  float m_wedgeStart;
  float m_wedgeEnd;

  MenuEntryType iconType;
  std::string m_activeName;
  std::string m_actualName;
  ci::Color m_activeColor;
  int m_defaultEntry;
  bool m_actionsOnly;

  static Vector2 m_windowSize;
  static float m_windowDiagonal;
  static float m_windowAspect;
  static Vector2 m_windowCenter;
  static std::vector<ci::gl::Texture> m_icons;

};

class UserInterface
{

public:

  UserInterface();
  void update(const std::vector<Vec4f>& _Tips, Sculpt* sculpt);
  void draw() const;
  void setWindowSize(const Vec2i& _Size);
  float maxActivation() const;
  void handleSelections(Sculpt* sculpt, LeapInteraction* leap, ThreeFormApp* app, Mesh* mesh);
  void setRegularFont(const ci::Font& font) { Menu::m_font = font; }
  void setBoldFont(const ci::Font& font) { Menu::m_boldFont = font; }
  void drawCursor(const ci::Vec2f& position, float opacity) const;

private:

  void initializeMenu(Menu& menu);

  static float angleOffsetForPosition(const Vector2& pos);

  std::vector<ci::Vec2f> _cursor_positions;

  Menu _type_menu;
  Menu _strength_menu;
  Menu _size_menu;
  Menu _color_menu;
  Menu _material_menu;
  Menu _spin_menu;
  Menu _wireframe_menu;
  Menu _symmetry_menu;
  Menu _history_menu;
  Menu _environment_menu;
  Menu _time_of_day_menu;
  Menu _main_menu;
  Menu _file_menu;
  Menu _sound_menu;

  bool _draw_color_menu;
  bool _first_selection_check;

};

#endif
