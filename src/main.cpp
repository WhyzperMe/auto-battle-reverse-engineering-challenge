#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <conio.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#if defined(_MSC_VER)
#define RE_NOINLINE __declspec(noinline)
#else
#define RE_NOINLINE __attribute__((noinline))
#endif

namespace Platform {

    class Console final {
    public:
        enum class Color {
            Default,
            Red,
            Green,
            Yellow,
            Cyan,
            Bold
        };

        bool Initialize() noexcept {
            const bool codePageSet = SetConsoleOutputCP(CP_UTF8) != 0 && SetConsoleCP(CP_UTF8) != 0;

            HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (outputHandle == INVALID_HANDLE_VALUE || outputHandle == nullptr) {
                ansiEnabled_ = false;
                return codePageSet;
            }

            DWORD mode = 0;
            if (!GetConsoleMode(outputHandle, &mode)) {
                ansiEnabled_ = false;
                return codePageSet;
            }

            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            ansiEnabled_ = SetConsoleMode(outputHandle, mode) != 0;
            return codePageSet && ansiEnabled_;
        }

        [[nodiscard]] const char* Code(Color color) const noexcept {
            if (!ansiEnabled_) {
                return "";
            }

            switch (color) {
            case Color::Red:    return "\033[31m";
            case Color::Green:  return "\033[32m";
            case Color::Yellow: return "\033[33m";
            case Color::Cyan:   return "\033[36m";
            case Color::Bold:   return "\033[1m";
            case Color::Default:
            default:            return "\033[0m";
            }
        }

        [[nodiscard]] const char* Reset() const noexcept {
            return Code(Color::Default);
        }

        static bool IsKeyAvailable() noexcept {
            return _kbhit() != 0;
        }

        static int ReadKey() noexcept {
            return _getch();
        }

    private:
        bool ansiEnabled_{ false };
    };

    class Clock final {
    public:
        static void SleepForMilliseconds(int milliseconds) {
            if (milliseconds <= 0) {
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }
    };

} // namespace Platform

namespace Logging {

    class Logger final {
    public:
        explicit Logger(const Platform::Console& console) noexcept
            : console_(console) {}

        template <typename... Args>
        void Info(const char* format, Args... args) const {
            Write(format, args...);
        }

        template <typename... Args>
        void Warning(const char* format, Args... args) const {
            std::printf("%s", console_.Code(Platform::Console::Color::Yellow));
            Write(format, args...);
            std::printf("%s", console_.Reset());
        }

        template <typename... Args>
        void Error(const char* format, Args... args) const {
            std::printf("%s", console_.Code(Platform::Console::Color::Red));
            Write(format, args...);
            std::printf("%s", console_.Reset());
        }

        [[nodiscard]] const Platform::Console& Console() const noexcept {
            return console_;
        }

    private:
        const Platform::Console& console_;

        template <typename... Args>
        static void Write(const char* format, Args... args) {
            if (format == nullptr) {
                return;
            }

            std::printf(format, args...);
        }
    };

} // namespace Logging

namespace Ui {

    constexpr int kTextWidth = 56;

    std::string FitText(std::string value) {
        if (value.size() > static_cast<std::size_t>(kTextWidth)) {
            value.resize(static_cast<std::size_t>(kTextWidth));
        }
        return value;
    }

    void PrintBoxLine(const Platform::Console& console, const std::string& text, Platform::Console::Color color) {
        const std::string clipped = FitText(text);
        std::printf(
            "%s║ %-*.*s ║%s\n",
            console.Code(color),
            kTextWidth,
            kTextWidth,
            clipped.c_str(),
            console.Reset()
        );
    }

    void PrintBox(const Platform::Console& console, const std::vector<std::string>& lines, Platform::Console::Color color) {
        std::printf("\n%s╔══════════════════════════════════════════════════════════╗%s\n", console.Code(color), console.Reset());
        for (const std::string& line : lines) {
            PrintBoxLine(console, line, color);
        }
        std::printf("%s╚══════════════════════════════════════════════════════════╝%s\n\n", console.Code(color), console.Reset());
    }

    void PrintTitle(const Platform::Console& console, const std::string& title, Platform::Console::Color color = Platform::Console::Color::Cyan) {
        PrintBox(console, { title }, color);
    }

} // namespace Ui

namespace Domain {

    struct SimulationConfig final {
        int heroHealth{ 100 };
        int heroDamage{ 8 };
        float heroSpeed{ 1.0f };

        int enemyHealth{ 180 };
        int enemyDamage{ 20 };
        float enemySpeed{ 1.5f };
        int enemyCount{ 5 };

        float attackRange{ 2.0f };
        float roamTargetReachedDistance{ 1.0f };
        int turnDelayMilliseconds{ 500 };
        int attackDelayMilliseconds{ 500 };

        int suggestedPatchedHeroHealth{ 9999 };
        int suggestedPatchedHeroDamage{ 100 };

        [[nodiscard]] std::optional<std::string> Validate() const {
            if (heroHealth <= 0 || enemyHealth <= 0) {
                return "Health values must be greater than zero.";
            }
            if (heroDamage <= 0 || enemyDamage <= 0) {
                return "Damage values must be greater than zero.";
            }
            if (heroSpeed <= 0.0f || enemySpeed <= 0.0f) {
                return "Movement speed values must be greater than zero.";
            }
            if (enemyCount <= 0 || enemyCount > 1000) {
                return "Enemy count must be between 1 and 1000.";
            }
            if (attackRange <= 0.0f || roamTargetReachedDistance <= 0.0f) {
                return "Distance thresholds must be greater than zero.";
            }
            if (turnDelayMilliseconds < 0 || turnDelayMilliseconds > 60000) {
                return "Turn delay must be between 0 and 60000 milliseconds.";
            }
            if (attackDelayMilliseconds < 0 || attackDelayMilliseconds > 60000) {
                return "Attack delay must be between 0 and 60000 milliseconds.";
            }
            return std::nullopt;
        }
    };

    class RandomGenerator final {
    public:
        RandomGenerator()
            : engine_(CreateSeed()) {}

        explicit RandomGenerator(std::uint32_t seed)
            : engine_(seed) {}

        [[nodiscard]] int Int(int minInclusive, int maxInclusive) {
            if (minInclusive > maxInclusive) {
                throw std::invalid_argument("Invalid integer random range.");
            }

            std::uniform_int_distribution<int> distribution(minInclusive, maxInclusive);
            return distribution(engine_);
        }

        [[nodiscard]] float Float(float minInclusive, float maxInclusive) {
            if (minInclusive > maxInclusive) {
                throw std::invalid_argument("Invalid floating-point random range.");
            }

            std::uniform_real_distribution<float> distribution(minInclusive, maxInclusive);
            return distribution(engine_);
        }

    private:
        std::mt19937 engine_;

        [[nodiscard]] static std::uint32_t CreateSeed() {
            std::random_device randomDevice;
            const auto now = static_cast<std::uint32_t>(
                std::chrono::high_resolution_clock::now().time_since_epoch().count()
                );
            return randomDevice() ^ now;
        }
    };

    struct Vector3 final {
        float x{ 0.0f };
        float y{ 0.0f };
        float z{ 0.0f };

        Vector3() = default;
        Vector3(float xValue, float yValue, float zValue) noexcept
            : x(xValue), y(yValue), z(zValue) {}

        [[nodiscard]] Vector3 operator+(const Vector3& other) const noexcept {
            return Vector3(x + other.x, y + other.y, z + other.z);
        }

        [[nodiscard]] Vector3 operator-(const Vector3& other) const noexcept {
            return Vector3(x - other.x, y - other.y, z - other.z);
        }

        [[nodiscard]] Vector3 operator*(float scalar) const noexcept {
            return Vector3(x * scalar, y * scalar, z * scalar);
        }

        [[nodiscard]] float LengthSquared() const noexcept {
            return (x * x) + (y * y) + (z * z);
        }

        [[nodiscard]] float Length() const noexcept {
            return std::sqrt(LengthSquared());
        }

        [[nodiscard]] float DistanceTo(const Vector3& other) const noexcept {
            return (*this - other).Length();
        }

        [[nodiscard]] Vector3 Normalized() const noexcept {
            constexpr float kEpsilon = 0.0001f;
            const float length = Length();
            if (length <= kEpsilon) {
                return Vector3();
            }

            return Vector3(x / length, y / length, z / length);
        }

        void Print() const {
            std::printf("(%.1f, %.1f, %.1f)", x, y, z);
        }
    };

    struct CombatStats final {
        int health{ 100 };
        int maxHealth{ 100 };
        int damage{ 10 };
        float speed{ 1.0f };
    };

    class Entity {
    public:
        static constexpr std::size_t kNameCapacity = 32;

        Entity(const char* name, CombatStats stats, Vector3 position)
            : name_(), position_(position), velocity_(), stats_(stats), isAlive_(true) {
            ValidateStats(stats_);
            SetName(name);
        }

        virtual ~Entity() = default;
        Entity(const Entity&) = default;
        Entity& operator=(const Entity&) = default;
        Entity(Entity&&) noexcept = default;
        Entity& operator=(Entity&&) noexcept = default;

        [[nodiscard]] const char* Name() const noexcept {
            return name_.data();
        }

        [[nodiscard]] const Vector3& Position() const noexcept {
            return position_;
        }

        [[nodiscard]] const Vector3& Velocity() const noexcept {
            return velocity_;
        }

        [[nodiscard]] int Health() const noexcept {
            return stats_.health;
        }

        [[nodiscard]] int MaxHealth() const noexcept {
            return stats_.maxHealth;
        }

        [[nodiscard]] int Damage() const noexcept {
            return stats_.damage;
        }

        [[nodiscard]] float Speed() const noexcept {
            return stats_.speed;
        }

        [[nodiscard]] bool IsAlive() const noexcept {
            return isAlive_;
        }

        [[nodiscard]] const void* HealthAddress() const noexcept {
            return &stats_.health;
        }

        [[nodiscard]] const void* MaxHealthAddress() const noexcept {
            return &stats_.maxHealth;
        }

        [[nodiscard]] const void* DamageAddress() const noexcept {
            return &stats_.damage;
        }

        void SetName(const char* value) noexcept {
            std::fill(name_.begin(), name_.end(), '\0');
            if (value == nullptr) {
                std::snprintf(name_.data(), name_.size(), "%s", "Unnamed");
                return;
            }

            std::snprintf(name_.data(), name_.size(), "%s", value);
        }

        void SetPosition(Vector3 value) noexcept {
            position_ = value;
        }

        RE_NOINLINE virtual void TakeDamage(int amount, const Logging::Logger& logger) {
            if (!isAlive_ || amount <= 0) {
                return;
            }

            stats_.health = amount >= stats_.health ? 0 : stats_.health - amount;
            if (stats_.health == 0) {
                isAlive_ = false;
                logger.Info(
                    "%s[%s] was defeated.%s\n",
                    logger.Console().Code(Platform::Console::Color::Red),
                    Name(),
                    logger.Console().Reset()
                );
            }
        }

        RE_NOINLINE virtual void Attack(Entity& target, RandomGenerator& random, const Logging::Logger& logger) {
            if (!isAlive_ || !target.IsAlive()) {
                return;
            }

            const int variance = random.Int(-2, 2);
            const int finalDamage = (std::max)(1, stats_.damage + variance);

            target.TakeDamage(finalDamage, logger);
            logger.Info(
                "%s[%s] attacks [%s] for %d damage.%s Target HP: %d/%d\n",
                logger.Console().Code(Platform::Console::Color::Yellow),
                Name(),
                target.Name(),
                finalDamage,
                logger.Console().Reset(),
                target.Health(),
                target.MaxHealth()
            );
        }

        void MoveTo(const Vector3& targetPosition) noexcept {
            if (!isAlive_) {
                return;
            }

            const Vector3 direction = (targetPosition - position_).Normalized();
            velocity_ = direction * stats_.speed;
            position_ = position_ + velocity_;
        }

        void PrintStatus(const Platform::Console& console) const {
            if (!isAlive_) {
                std::printf("%s[%s] is DEAD%s\n", console.Code(Platform::Console::Color::Red), Name(), console.Reset());
                return;
            }

            constexpr int kBarLength = 20;
            const float ratio = stats_.maxHealth > 0
                ? static_cast<float>(stats_.health) / static_cast<float>(stats_.maxHealth)
                : 0.0f;
            const float boundedRatio = (std::clamp)(ratio, 0.0f, 1.0f);
            const int filledSegments = static_cast<int>(boundedRatio * static_cast<float>(kBarLength));

            std::printf(
                "%s[%s]%s HP: %4d/%4d | DMG: %3d | SPD: %.1f | POS: ",
                console.Code(Platform::Console::Color::Cyan),
                Name(),
                console.Reset(),
                stats_.health,
                stats_.maxHealth,
                stats_.damage,
                stats_.speed
            );
            position_.Print();
            std::printf("\n     HP [");

            for (int i = 0; i < kBarLength; ++i) {
                if (i < filledSegments) {
                    if (boundedRatio > 0.5f) {
                        std::printf("%s█%s", console.Code(Platform::Console::Color::Green), console.Reset());
                    }
                    else if (boundedRatio > 0.25f) {
                        std::printf("%s█%s", console.Code(Platform::Console::Color::Yellow), console.Reset());
                    }
                    else {
                        std::printf("%s█%s", console.Code(Platform::Console::Color::Red), console.Reset());
                    }
                }
                else {
                    std::printf("░");
                }
            }

            std::printf("]\n");
        }

    protected:
        static void ValidateStats(const CombatStats& stats) {
            if (stats.health <= 0 || stats.maxHealth <= 0 || stats.damage <= 0 || stats.speed <= 0.0f) {
                throw std::invalid_argument("Invalid entity combat stats.");
            }
            if (stats.health > stats.maxHealth) {
                throw std::invalid_argument("Entity health cannot exceed maxHealth.");
            }
        }

    private:
        // A fixed-size name buffer and primitive combat fields are intentional for this educational
        // reverse-engineering challenge. The fields remain simple to inspect in x64dbg and Ghidra.
        std::array<char, kNameCapacity> name_{};
        Vector3 position_{};
        Vector3 velocity_{};
        CombatStats stats_{};
        bool isAlive_{ true };
    };

    class Hero final : public Entity {
    public:
        explicit Hero(const SimulationConfig& config)
            : Entity(
                "HERO",
                CombatStats{ config.heroHealth, config.heroHealth, config.heroDamage, config.heroSpeed },
                Vector3(0.0f, 0.0f, 0.0f)
            ) {}

        void ClearTarget() noexcept {
            targetEnemyIndex_.reset();
            alertMode_ = false;
        }

        void SetTarget(std::size_t enemyIndex) noexcept {
            targetEnemyIndex_ = enemyIndex;
            alertMode_ = true;
        }

        [[nodiscard]] std::optional<std::size_t> TargetEnemyIndex() const noexcept {
            return targetEnemyIndex_;
        }

        [[nodiscard]] bool IsInAlertMode() const noexcept {
            return alertMode_;
        }

    private:
        std::optional<std::size_t> targetEnemyIndex_{};
        bool alertMode_{ false };
    };

    class Enemy final : public Entity {
    public:
        enum class State {
            Roaming,
            Aggressive,
            Dead
        };

        Enemy(const char* name, const SimulationConfig& config, Vector3 spawnPosition, RandomGenerator& random)
            : Entity(
                name,
                CombatStats{ config.enemyHealth, config.enemyHealth, config.enemyDamage, config.enemySpeed },
                spawnPosition
            ) {
            GenerateRoamTarget(random);
        }

        [[nodiscard]] State CurrentState() const noexcept {
            return state_;
        }

        void SetAggressive(const Logging::Logger& logger) {
            if (!IsAlive() || state_ == State::Aggressive) {
                return;
            }

            state_ = State::Aggressive;
            logger.Warning("! [%s] is now aggressive.\n", Name());
        }

        void GenerateRoamTarget(RandomGenerator& random) {
            roamTarget_ = Vector3(
                random.Float(-20.0f, 20.0f),
                0.0f,
                random.Float(-20.0f, 20.0f)
            );
        }

        [[nodiscard]] bool Update(Hero& hero, const SimulationConfig& config, RandomGenerator& random, const Logging::Logger& logger) {
            if (!IsAlive()) {
                state_ = State::Dead;
                return false;
            }

            switch (state_) {
            case State::Roaming:
                if (Position().DistanceTo(roamTarget_) < config.roamTargetReachedDistance) {
                    GenerateRoamTarget(random);
                }
                MoveTo(roamTarget_);
                return false;

            case State::Aggressive:
                return UpdateAggressiveState(hero, config, random, logger);

            case State::Dead:
                return false;
            }

            return false;
        }

    private:
        State state_{ State::Roaming };
        Vector3 roamTarget_{};

        [[nodiscard]] bool UpdateAggressiveState(Hero& hero, const SimulationConfig& config, RandomGenerator& random, const Logging::Logger& logger) {
            if (!hero.IsAlive()) {
                state_ = State::Roaming;
                return false;
            }

            const float distanceToHero = Position().DistanceTo(hero.Position());
            if (distanceToHero < config.attackRange) {
                Attack(hero, random, logger);
                return true;
            }

            MoveTo(hero.Position());
            return false;
        }
    };

    struct MemoryProbe final {
        const void* objectAddress{ nullptr };
        const void* healthAddress{ nullptr };
        const void* maxHealthAddress{ nullptr };
        const void* damageAddress{ nullptr };
        std::size_t healthOffset{ 0 };
        std::size_t maxHealthOffset{ 0 };
        std::size_t damageOffset{ 0 };
    };

    [[nodiscard]] std::size_t ByteOffsetFromObject(const void* objectAddress, const void* fieldAddress) noexcept {
        const auto object = reinterpret_cast<std::uintptr_t>(objectAddress);
        const auto field = reinterpret_cast<std::uintptr_t>(fieldAddress);
        return static_cast<std::size_t>(field - object);
    }

    [[nodiscard]] MemoryProbe CreateMemoryProbe(const Hero& hero) noexcept {
        MemoryProbe probe{};
        probe.objectAddress = static_cast<const void*>(&hero);
        probe.healthAddress = hero.HealthAddress();
        probe.maxHealthAddress = hero.MaxHealthAddress();
        probe.damageAddress = hero.DamageAddress();
        probe.healthOffset = ByteOffsetFromObject(probe.objectAddress, probe.healthAddress);
        probe.maxHealthOffset = ByteOffsetFromObject(probe.objectAddress, probe.maxHealthAddress);
        probe.damageOffset = ByteOffsetFromObject(probe.objectAddress, probe.damageAddress);
        return probe;
    }

} // namespace Domain

namespace App {

    class ChallengeHintSystem final {
    public:
        ChallengeHintSystem(const Domain::SimulationConfig& config, const Domain::Hero& hero, const Logging::Logger& logger) noexcept
            : config_(config), hero_(hero), logger_(logger) {}

        void PrintIntro() const {
            logger_.Info(
                "%sHint mode:%s direct answers are hidden. Press [H] during the simulation to reveal one hint at a time.\n",
                logger_.Console().Code(Platform::Console::Color::Cyan),
                logger_.Console().Reset()
            );
        }

        void RevealNext() {
            constexpr std::size_t kHintCount = 8;

            if (nextHintIndex_ >= kHintCount) {
                logger_.Warning("\nAll reverse-engineering hints have already been revealed.\n");
                return;
            }

            const Domain::MemoryProbe probe = Domain::CreateMemoryProbe(hero_);
            logger_.Info(
                "\n%sReverse-engineering hint %zu/%zu:%s\n",
                logger_.Console().Code(Platform::Console::Color::Cyan),
                nextHintIndex_ + 1,
                kHintCount,
                logger_.Console().Reset()
            );

            switch (nextHintIndex_) {
            case 0:
                logger_.Info("  + Observe the baseline behavior first: the hero loses because incoming damage outpaces outgoing damage.\n");
                break;
            case 1:
                logger_.Info("  + Search for the ASCII string 'HERO' in process memory, then inspect nearby object data.\n");
                break;
            case 2:
                logger_.Info("  + Set breakpoints on Entity::TakeDamage and Entity::Attack to observe combat reads and writes.\n");
                break;
            case 3:
                logger_.Info("  + In Ghidra, inspect references to combat-field reads and writes. The relevant values are simple 32-bit fields.\n");
                break;
            case 4:
                logger_.Info("  + Runtime HERO object address in this run: %p\n", probe.objectAddress);
                break;
            case 5:
                logger_.Info(
                    "  + HERO.health    address: %p | offset: +0x%llX\n",
                    probe.healthAddress,
                    static_cast<unsigned long long>(probe.healthOffset)
                );
                break;
            case 6:
                logger_.Info(
                    "  + HERO.maxHealth address: %p | offset: +0x%llX\n",
                    probe.maxHealthAddress,
                    static_cast<unsigned long long>(probe.maxHealthOffset)
                );
                break;
            case 7:
                logger_.Info(
                    "  + HERO.damage    address: %p | offset: +0x%llX. Suggested training patch: HP=%d, DMG=%d.\n",
                    probe.damageAddress,
                    static_cast<unsigned long long>(probe.damageOffset),
                    config_.suggestedPatchedHeroHealth,
                    config_.suggestedPatchedHeroDamage
                );
                break;
            default:
                break;
            }

            ++nextHintIndex_;
        }

    private:
        const Domain::SimulationConfig& config_;
        const Domain::Hero& hero_;
        const Logging::Logger& logger_;
        std::size_t nextHintIndex_{ 0 };
    };

    class GameManager final {
    public:
        GameManager(Domain::SimulationConfig config, const Logging::Logger& logger)
            : config_(std::move(config)), logger_(logger), hero_(config_), hints_(config_, hero_, logger_) {
            if (const std::optional<std::string> error = config_.Validate()) {
                throw std::invalid_argument(*error);
            }
        }

        GameManager(const GameManager&) = delete;
        GameManager& operator=(const GameManager&) = delete;

        [[nodiscard]] const Domain::Hero& HeroForInspection() const noexcept {
            return hero_;
        }

        void Initialize() {
            Ui::PrintTitle(logger_.Console(), "GAME INITIALIZATION");

            logger_.Info("%sHero stats:%s\n", logger_.Console().Code(Platform::Console::Color::Green), logger_.Console().Reset());
            logger_.Info("  + Health: %d\n", hero_.Health());
            logger_.Info("  + Damage: %d\n", hero_.Damage());
            logger_.Info("  + Speed:  %.1f\n\n", hero_.Speed());

            logger_.Info(
                "%sSpawning %d enemies...%s\n",
                logger_.Console().Code(Platform::Console::Color::Yellow),
                config_.enemyCount,
                logger_.Console().Reset()
            );

            enemies_.clear();
            enemies_.reserve(static_cast<std::size_t>(config_.enemyCount));

            for (int index = 0; index < config_.enemyCount; ++index) {
                const std::string enemyName = "Monster_" + std::to_string(index + 1);
                const Domain::Vector3 spawnPosition(
                    random_.Float(-15.0f, 15.0f),
                    0.0f,
                    random_.Float(-15.0f, 15.0f)
                );

                enemies_.emplace_back(enemyName.c_str(), config_, spawnPosition, random_);

                logger_.Info("  + [%s] spawned at ", enemies_.back().Name());
                enemies_.back().Position().Print();
                logger_.Info(" [HP: %d, DMG: %d]\n", enemies_.back().Health(), enemies_.back().Damage());
            }

            Ui::PrintTitle(logger_.Console(), "BATTLE START");
            logger_.Info("%sControls:%s\n", logger_.Console().Code(Platform::Console::Color::Yellow), logger_.Console().Reset());
            logger_.Info("  + [SPACE] Print status\n");
            logger_.Info("  + [H]     Reveal next reverse-engineering hint\n");
            logger_.Info("  + [ESC]   Exit simulation\n");
            logger_.Info("  + Combat pacing: %d ms pause after every attack\n\n", config_.attackDelayMilliseconds);
            hints_.PrintIntro();
            logger_.Info("\n");
        }

        void Run() {
            while (!gameOver_) {
                ++turnCount_;

                SelectNextTargetIfNeeded();
                UpdateHeroAction();
                if (gameOver_ || CheckEndConditions()) {
                    break;
                }

                UpdateEnemies();
                if (gameOver_ || CheckEndConditions()) {
                    break;
                }

                WaitWithInputPolling(config_.turnDelayMilliseconds);
            }
        }

        void PrintStatus() const {
            Ui::PrintBox(
                logger_.Console(),
                { "STATUS - Turn " + std::to_string(turnCount_) },
                Platform::Console::Color::Cyan
            );

            logger_.Info("%s=== HERO ===%s\n", logger_.Console().Code(Platform::Console::Color::Green), logger_.Console().Reset());
            hero_.PrintStatus(logger_.Console());

            logger_.Info("\n%s=== ENEMIES ===%s\n", logger_.Console().Code(Platform::Console::Color::Red), logger_.Console().Reset());
            for (const Domain::Enemy& enemy : enemies_) {
                enemy.PrintStatus(logger_.Console());
            }
            logger_.Info("\n");
        }

    private:
        Domain::SimulationConfig config_{};
        const Logging::Logger& logger_;
        Domain::RandomGenerator random_{};
        Domain::Hero hero_;
        ChallengeHintSystem hints_;
        std::vector<Domain::Enemy> enemies_{};
        int turnCount_{ 0 };
        bool gameOver_{ false };

        void SelectNextTargetIfNeeded() {
            const std::optional<std::size_t> targetIndex = hero_.TargetEnemyIndex();
            if (targetIndex.has_value() && *targetIndex < enemies_.size() && enemies_[*targetIndex].IsAlive()) {
                return;
            }

            hero_.ClearTarget();

            for (std::size_t index = 0; index < enemies_.size(); ++index) {
                if (enemies_[index].IsAlive()) {
                    hero_.SetTarget(index);
                    logger_.Info(
                        "\n%s* [HERO] selected [%s] as the current target.%s\n",
                        logger_.Console().Code(Platform::Console::Color::Green),
                        enemies_[index].Name(),
                        logger_.Console().Reset()
                    );
                    AlertAllEnemies();
                    return;
                }
            }
        }

        void AlertAllEnemies() {
            logger_.Warning("! ALERT: all living enemies become aggressive.\n\n");
            for (Domain::Enemy& enemy : enemies_) {
                enemy.SetAggressive(logger_);
            }
        }

        void UpdateHeroAction() {
            const std::optional<std::size_t> targetIndex = hero_.TargetEnemyIndex();
            if (!hero_.IsAlive() || !targetIndex.has_value() || *targetIndex >= enemies_.size()) {
                return;
            }

            Domain::Enemy& target = enemies_[*targetIndex];
            if (!target.IsAlive()) {
                hero_.ClearTarget();
                return;
            }

            const float distanceToTarget = hero_.Position().DistanceTo(target.Position());
            if (distanceToTarget < config_.attackRange) {
                hero_.Attack(target, random_, logger_);
                PauseAfterAttackIfCombatContinues();
            }
            else {
                hero_.MoveTo(target.Position());
            }
        }

        void UpdateEnemies() {
            for (Domain::Enemy& enemy : enemies_) {
                if (!hero_.IsAlive() || gameOver_) {
                    break;
                }

                const bool enemyAttacked = enemy.Update(hero_, config_, random_, logger_);
                if (enemyAttacked) {
                    PauseAfterAttackIfCombatContinues();
                }
            }
        }

        [[nodiscard]] bool HasLivingEnemies() const {
            return std::any_of(
                enemies_.begin(),
                enemies_.end(),
                [](const Domain::Enemy& enemy) { return enemy.IsAlive(); }
            );
        }

        void PauseAfterAttackIfCombatContinues() {
            if (!hero_.IsAlive() || !HasLivingEnemies()) {
                return;
            }

            WaitWithInputPolling(config_.attackDelayMilliseconds);
        }

        void WaitWithInputPolling(int milliseconds) {
            if (milliseconds <= 0) {
                HandleKeyboardInput();
                return;
            }

            constexpr int kPollingIntervalMilliseconds = 25;
            int elapsedMilliseconds = 0;

            while (!gameOver_ && elapsedMilliseconds < milliseconds) {
                const int remainingMilliseconds = milliseconds - elapsedMilliseconds;
                const int sleepMilliseconds = (std::min)(kPollingIntervalMilliseconds, remainingMilliseconds);
                Platform::Clock::SleepForMilliseconds(sleepMilliseconds);
                elapsedMilliseconds += sleepMilliseconds;
                HandleKeyboardInput();
            }
        }

        [[nodiscard]] bool CheckEndConditions() {
            if (!hero_.IsAlive()) {
                Ui::PrintBox(
                    logger_.Console(),
                    {
                        "GAME OVER",
                        "The hero was defeated after " + std::to_string(turnCount_) + " turns."
                    },
                    Platform::Console::Color::Red
                );
                gameOver_ = true;
                return true;
            }

            const bool allEnemiesDefeated = std::all_of(
                enemies_.begin(),
                enemies_.end(),
                [](const Domain::Enemy& enemy) { return !enemy.IsAlive(); }
            );

            if (allEnemiesDefeated) {
                Ui::PrintBox(
                    logger_.Console(),
                    {
                        "VICTORY",
                        "The hero defeated all enemies in " + std::to_string(turnCount_) + " turns."
                    },
                    Platform::Console::Color::Green
                );
                gameOver_ = true;
                return true;
            }

            return false;
        }

        void HandleKeyboardInput() {
            if (!Platform::Console::IsKeyAvailable()) {
                return;
            }

            const int key = Platform::Console::ReadKey();
            if (key == ' ') {
                PrintStatus();
            }
            else if (key == 'h' || key == 'H') {
                hints_.RevealNext();
            }
            else if (key == 27) {
                gameOver_ = true;
                logger_.Warning("\nSimulation aborted by user.\n");
            }
        }
    };

    void PrintSplashScreen(const Platform::Console& console) {
        Ui::PrintBox(
            console,
            {
                "AUTO-BATTLE SIMULATION",
                "Reverse Engineering Challenge"
            },
            Platform::Console::Color::Cyan
        );
    }

    void PrintChallengeInstructions(const Domain::SimulationConfig& config, const Logging::Logger& logger) {
        logger.Info("%sObjective:%s\n", logger.Console().Code(Platform::Console::Color::Yellow), logger.Console().Reset());
        logger.Info("  1. Build this project as x64 Debug with compiler optimizations disabled.\n");
        logger.Info("  2. Start the executable in x64dbg.\n");
        logger.Info("  3. Locate the HERO object and identify the nearby combat fields.\n");
        logger.Info("  4. Increase the hero's health and damage at runtime.\n");
        logger.Info("  5. Resume execution and verify that the hero wins the fight.\n");

        logger.Info("\n%sReverse-engineering hints:%s\n", logger.Console().Code(Platform::Console::Color::Cyan), logger.Console().Reset());
        logger.Info("  + Direct answers are hidden at startup.\n");
        logger.Info("  + Press [H] during the simulation to reveal one hint at a time.\n");
        logger.Info("  + Final hints include dynamic addresses and offsets for this process run.\n");
        logger.Info("  + Suggested training patch values are intentionally revealed only in the last hint.\n");

        logger.Info("\nStarting simulation in 3 seconds...\n");
        (void)config;
    }

} // namespace App

int main() {
    try {
        Platform::Console console;
        const bool consoleReady = console.Initialize();
        const Logging::Logger logger(console);

        if (!consoleReady) {
            logger.Warning("Warning: UTF-8 or ANSI console support could not be fully enabled. Continuing without rich console support.\n");
        }

        const Domain::SimulationConfig config{};
        if (const std::optional<std::string> configError = config.Validate()) {
            logger.Error("Configuration error: %s\n", configError->c_str());
            return EXIT_FAILURE;
        }

        App::PrintSplashScreen(console);

        App::GameManager game(config, logger);
        App::PrintChallengeInstructions(config, logger);

        Platform::Clock::SleepForMilliseconds(3000);
        game.Initialize();
        game.Run();

        logger.Info("\n%sPress any key to exit...%s\n", console.Code(Platform::Console::Color::Yellow), console.Reset());
        Platform::Console::ReadKey();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception) {
        std::fprintf(stderr, "Fatal error: %s\n", exception.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        std::fprintf(stderr, "Fatal error: unknown exception.\n");
        return EXIT_FAILURE;
    }
}
