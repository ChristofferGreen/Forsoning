#pragma once

#include <functional>
#include <filesystem>
#include <optional>

namespace Forsoning {

struct Data {
    Data(int d) {}
};

struct PathSpaceTE {
	PathSpaceTE() = default;
	template<typename T>
	PathSpaceTE(T x)                    : self(std::make_unique<model<T>>(std::move(x))) {}
	PathSpaceTE(PathSpaceTE const &rhs) : self(rhs.self->copy_())                        {}

	PathSpaceTE& operator=(PathSpaceTE const &rhs) {return *this = PathSpaceTE(rhs);}
	PathSpaceTE& operator=(PathSpaceTE&&) noexcept = default;

	auto insert(std::filesystem::path const &path, Data const &data) const -> void {this->self->insert_(path, data);}
private:
	struct concept_t {
		virtual ~concept_t() = default;
		
		virtual auto copy_()                                                      const -> std::unique_ptr<concept_t> = 0;
		virtual auto insert_(std::filesystem::path const &path, Data const &data) const -> void                       = 0;
	};
	template<typename T> 
	struct model final : concept_t {
		model(T x) : data(std::move(x)) {}

		auto copy_() const -> std::unique_ptr<concept_t> override {return std::make_unique<model>(*this);}
		
		auto insert_(std::filesystem::path const &path, Data const &d) const -> void override {return this->data.insert(path, d);}
		
		T data;
	};
	std::unique_ptr<concept_t> self;
};

struct PathSpace {
    friend PathSpaceTE;
    protected:
     auto insert(std::filesystem::path const &path, Data const &data) const -> void {};
};

struct View {
    View(PathSpaceTE path, std::function<bool()> fun) {};
    
    auto insert(std::filesystem::path const &path, Data const &data) const -> void {};
    template<typename T>
    auto grab(std::filesystem::path const &path) const -> std::optional<T> { return {}; };
};

}