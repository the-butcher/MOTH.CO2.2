/** Allows Create React App to be configured. Use only if absolutely necessary. */

const { override, addExternalBabelPlugins } = require("customize-cra");

module.exports = {
    webpack: override(
        ...addExternalBabelPlugins(
            "@babel/plugin-proposal-nullish-coalescing-operator",
            "@babel/plugin-proposal-optional-chaining"
        )
    ),
};
