const path = require('path'),
    webpack = require('webpack'),
    HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  entry: "./src/index.tsx",
  output: {
    filename: "bundle.js",
    path: __dirname + "/dist"
  },

  devtool: "source-map",
  mode: "development",

  resolve: {
    extensions: [".ts", ".tsx", ".js", ".json"]
  },

  module: {
    rules: [
      { test: /\.tsx?$/, loader: "awesome-typescript-loader" },
      { enforce: "pre", test: /\.js$/, loader: "source-map-loader" },
      { test: /\.sass$/,
        use: [{
            loader: "style-loader"
          }, {
            loader: "css-loader", options: {
              sourceMap: true
            }
          }, {
            loader: "sass-loader", options: {
            sourceMap: true
          }
        }]
      }
    ]
  },
  plugins: [
    new HtmlWebpackPlugin({ template: path.resolve(__dirname, 'src', 'index.html') }),
    new webpack.HotModuleReplacementPlugin()
  ]
};
