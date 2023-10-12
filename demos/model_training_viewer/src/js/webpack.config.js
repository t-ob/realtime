const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const WasmPackPlugin = require("@wasm-tool/wasm-pack-plugin");

module.exports = {
  mode: 'development',
  entry: './src/index.js',
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'bundle.js',
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: 'src/index.html',
      inject: "body"
    }),
    new WasmPackPlugin({
      crateDirectory: path.resolve(__dirname, '../rs/receiver'),
      outDir: path.resolve(__dirname, './pkg')
    }),
  ],
  module: {
    rules: [
      {
        test: /\.wasm$/,
        type: 'webassembly/async',
      },
    ],
  },
  experiments: {
    asyncWebAssembly: true,
  },
  devServer: {
    allowedHosts: "all",
    static: path.join(__dirname, 'dist'),
    host: "0.0.0.0",
    proxy: {
      "/ws": {
        target: "ws://0.0.0.0:9001",
        ws: true
      }
    }
  },
};
