#include "mellohi/core/assets/toml_asset.hpp"

namespace mellohi
{
    TomlAsset::TomlAsset(std::shared_ptr<AssetManager> asset_manager_ptr, const AssetId &asset_id)
        : Asset(asset_manager_ptr, asset_id)
    {

    }

    toml::table TomlAsset::parse_toml_table() const
    {
        return toml::parse(get_id().read_file_as_string());
    }

    template<>
    std::optional<AssetId> TomlAsset::parse_opt(const toml::table &table, std::string_view path) const
    {
        return parse_opt<std::string>(table, path).transform([](const auto &str) { return AssetId(str); });
    }

    template<>
    std::optional<Color> TomlAsset::parse_opt(const toml::table &table, std::string_view path) const
    {
        const auto value_array_ptr = table[toml::path(path)].as_array();
        if (value_array_ptr && value_array_ptr->size() >= 3 && value_array_ptr->size() <= 4)
        {
            const auto value_r_opt = value_array_ptr->at(0).value<f32>();
            const auto value_g_opt = value_array_ptr->at(1).value<f32>();
            const auto value_b_opt = value_array_ptr->at(2).value<f32>();

            if (value_r_opt.has_value() && value_g_opt.has_value() && value_b_opt.has_value())
            {
                const auto value_a_ptr = value_array_ptr->get(3);

                if (value_a_ptr)
                {
                    const auto value_a_opt = value_a_ptr->value<f32>();

                    if (value_a_opt.has_value())
                    {
                        return Color(value_r_opt.value(), value_g_opt.value(), value_b_opt.value(), value_a_opt.value());
                    }
                }

                return Color(value_r_opt.value(), value_g_opt.value(), value_b_opt.value());
            }
        }

        const auto value_string_opt = table[toml::path(path)].value<std::string>();
        if (value_string_opt.has_value())
        {
            return Color(value_string_opt.value());
        }

        return std::nullopt;
    }

    template<>
    std::optional<uvec2> TomlAsset::parse_opt(const toml::table &table, std::string_view path) const
    {
        const auto value_array = table[toml::path(path)].as_array();
        if (value_array && value_array->size() == 2)
        {
            const auto value_x_opt = value_array->at(0).value<u32>();
            const auto value_y_opt = value_array->at(1).value<u32>();

            if (value_x_opt.has_value() && value_y_opt.has_value())
            {
                return uvec2(value_x_opt.value(), value_y_opt.value());
            }
        }

        return std::nullopt;
    }
}
