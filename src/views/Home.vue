<template>
  <div class="home">
    <v-container>
      <v-row>
        <v-col-6>
          <div class="title-box">
            <div class="title">
              Last Update:
              <p id="intext">{{ this.last_update }}</p>
            </div>
          </div>
        </v-col-6>
        <v-col>
          <div class="automode">
            <v-sheet
              width="100%"
              height="100%"
              class="pa-6"
              style="background-color: rgba(250, 235, 215, 0)"
            >
              <v-switch
                v-model="automode"
                hide-details
                inset
                :disabled="toggling"
                :label="`${mode}`"
                @change="test()"
              >
              </v-switch>
            </v-sheet>
            <v-fade-transition hide-on-leave>
              <v-card
                v-if="toggling"
                append-icon="$close"
                class="toggle_card"
                elevation="16"
                height="200"
                width="200"
                color="#fff"
              >
                <v-card-text> Test </v-card-text>
                <v-btn
                  class="btn"
                  color="primary"
                  max-height="50"
                  width="80"
                  text
                  @click="toggling = false"
                >
                  I accept
                </v-btn>
              </v-card>
            </v-fade-transition>
          </div>
        </v-col>
      </v-row>
      <v-row>
        <v-col-6>
          <div class="title-box">
            <div class="title">
              Temperature:
              <p id="intext">{{ this.temperature }}°C</p>
            </div>
          </div>
        </v-col-6>
        <v-col>
          <v-form>
            <v-textarea
              label="Input"
              auto-grow
              outlined
              rows="1"
              row-height="15"
              v-model="setPwm"
            ></v-textarea>
            <v-btn>Send</v-btn>
          </v-form>
        </v-col>
      </v-row>
      <v-row>
        <v-col>
          <div class="title">
            FAN pwm:
            <p id="intext">{{ this.pwm }}</p>
          </div>
        </v-col>
      </v-row>
      <v-row>
        <v-col>
          <div class="bbtn">
            <v-btn :disabled="btnDisable" @click="clk">Refresh</v-btn>
          </div>
        </v-col>
      </v-row>
    </v-container>
  </div>
</template>

<script>
// @ is an alias to /src

export default {
  name: "Home",
  data() {
    return {
      model: true,
      temperature: 0,
      last_update: 0,
      pwm: 0,
      automode: null,
      btnDisable: false,
      toggling: false,
      setPwm: 0,
    };
  },
  computed: {
    mode() {
      return this.automode ? "Auto Mode" : "Manual Mode";
    },
  },
  watch: {},
  components: {},
  methods: {
    clk() {
      this.$axios
        .get(`https://wemosd1dp.azurewebsites.net/`)
        .then((result) => {
          console.log(result);
          this.temperature = result.data.temperature.toFixed(2);
          this.last_update = result.data.last_update;
          this.pwm = result.data.pwm;
          this.automode = result.data.automode;
        })
        .catch((err) => {
          console.log(err);
        });
      // this.btnDisable = !this.btnDisable
    },

    test() {
      this.toggling = true;
      let mode = this.automode ? 1 : 0;
      console.log("mode:", mode);
      this.$axios
        .post(`https://localhost:80/api/changemode`, {
          mode: mode,
        })
        .then((result) => {
          console.log(result);
          this.temperature = result.data.temperature.toFixed(2);
          this.last_update = result.data.last_update;
          this.pwm = result.data.pwm;
          this.automode = result.data.automode;
          setTimeout(() => {
            this.toggling = false;
          }, 500);
        })
        .catch((err) => {
          this.automode = !this.automode;
          setTimeout(() => {
            this.toggling = false;
          }, 500);
          console.log(err);
        });
    },
  },
  created() {
    console.log("=== Home created! === ");
    this.$axios
      .get(`https://wemosd1dp.azurewebsites.net/`)
      .then((result) => {
        this.temperature = result.data.temperature.toFixed(2);
        this.last_update = result.data.last_update;
        this.pwm = result.data.pwm;
        this.automode = result.data.automode;
      })
      .catch((err) => {
        console.log(err);
      });
  },
  beforeUpdate() {
    console.log("=== Home beforeUpdate! === ");
  },
  updated() {
    console.log("=== Home updated! === ");
  },
};
</script>
<style lang="scss" scoped>
* {
  // border: 1px solid #f00;
  color: #fff;
}

.home {
  width: 100vw;
  height: 100vh;
  padding-top: 50px;
  // position: relative;
  // background-color: white;
  background-image: linear-gradient(
    0deg,
    rgb(68, 137, 194) 0%,
    rgb(18, 99, 136) 100%
  );
  background-size: cover;
}

.title {
  color: rgb(255, 255, 255);
}

#intext {
  font-size: 20px;
}

.bbtn {
  border-radius: 5px;
  height: 25px;
  // margin-left: 40px;
}

.toggle_card {
  position: absolute;
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  z-index: 99;
  // height: auto;
  // margin-top: 50px;
}

.automode {
  // position: absolute;
  .v-input--selection-controls {
    margin-top: 0;
  }
  // margin-left: 100px;
}
.title-box {
  padding: 12px;
}
.btn {
  position: relative;
  // right: 50%;
  top: 40%;
  left: 30%;
}
</style>
